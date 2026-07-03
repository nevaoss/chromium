// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/unexportable_keys/unexportable_key_service_impl.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <variant>

#include "base/containers/map_util.h"
#include "base/containers/to_vector.h"
#include "base/feature_list.h"
#include "base/functional/bind.h"
#include "base/functional/callback.h"
#include "base/logging.h"
#include "base/memory/scoped_refptr.h"
#include "base/memory/weak_ptr.h"
#include "base/notimplemented.h"
#include "base/notreached.h"
#include "base/types/expected.h"
#include "base/types/expected_macros.h"
#include "build/build_config.h"
#include "build/buildflag.h"
#include "components/unexportable_keys/background_task_origin.h"
#include "components/unexportable_keys/features.h"
#include "components/unexportable_keys/service_error.h"
#include "components/unexportable_keys/unexportable_key_id.h"
#include "components/unexportable_keys/unexportable_key_task_manager.h"
#include "crypto/unexportable_key.h"
#include "third_party/abseil-cpp/absl/container/flat_hash_map.h"
#include "third_party/abseil-cpp/absl/container/hash_container_defaults.h"

namespace unexportable_keys {

namespace {

using WrappedKeyAndTag = std::pair<std::vector<uint8_t>, std::string>;
using WrappedKeyAndTagView =
    std::pair<base::span<const uint8_t>, std::string_view>;

struct WrappedKeyAndTagViewHash
    : absl::DefaultHashContainerHash<WrappedKeyAndTagView> {
  using is_transparent = void;
};

WrappedKeyAndTag Materialize(WrappedKeyAndTagView view) {
  auto [wrapped_key, tag] = view;
  return {base::ToVector(wrapped_key), std::string(tag)};
}

// Returns the application tag from the config on Mac if the provider supports
// stateful unexportable keys. Otherwise, returns an empty string.
std::string_view GetApplicationTag(
    const crypto::UnexportableKeyProvider::Config& config) {
#if BUILDFLAG(IS_MAC)
  if (UnexportableKeyServiceImpl::IsStatefulUnexportableKeyProviderSupported(
          config)) {
    return config.application_tag;
  }

  return "";
#else
  return "";
#endif  // BUILDFLAG(IS_MAC)
}

// Convenience method to create a `WrappedKeyAndTag` from a
// `RefCountedUnexportableKey`.
std::pair<std::vector<uint8_t>, std::string> GetWrappedKeyAndTag(
    const RefCountedUnexportableKey& key) {
  std::string tag;
  if (const crypto::StatefulKey* stateful_key = key.key().AsStatefulKey()) {
    tag = stateful_key->GetKeyTag();
  }

  return {key.key().GetWrappedKey(), std::move(tag)};
}

// Class holding either a `KeyIdType` or a list of callbacks waiting for the
// key creation.
template <typename KeyIdType>
class MaybePendingKeyId {
 public:
  using CallbackType = base::OnceCallback<void(ServiceErrorOr<KeyIdType>)>;
  using PendingCallbacks = std::vector<CallbackType>;
  using PendingCallbacksOrKeyId = std::variant<PendingCallbacks, KeyIdType>;

  // Constructs an instance holding a list of callbacks.
  MaybePendingKeyId() = default;
  MaybePendingKeyId(MaybePendingKeyId&&) = default;
  MaybePendingKeyId& operator=(MaybePendingKeyId&&) = default;

  // Constructs an instance holding `key_id`.
  explicit MaybePendingKeyId(KeyIdType key_id)
      : pending_callbacks_or_key_id_(key_id) {}

  ~MaybePendingKeyId() {
    if (!HasKeyId()) {
      RunCallbacksWithFailure(ServiceError::kOperationCancelled);
    }
  }

  // Returns true if a key has been assigned to this instance. Otherwise,
  // returns false which means that this instance holds a list of callbacks.
  bool HasKeyId() const {
    return std::holds_alternative<KeyIdType>(pending_callbacks_or_key_id_);
  }

  // This method should be called only if `HasKeyId()` is true.
  KeyIdType GetKeyId() const {
    CHECK(HasKeyId());
    return std::get<KeyIdType>(pending_callbacks_or_key_id_);
  }

  // These methods should be called only if `HasKeyId()` is false.

  // Adds `callback` to the list of callbacks and returns size of the list.
  size_t AddCallback(CallbackType callback) {
    CHECK(!HasKeyId());
    GetCallbacks().push_back(std::move(callback));
    return GetCallbacks().size();
  }

  void SetKeyIdAndRunCallbacks(KeyIdType key_id) {
    CHECK(!HasKeyId());
    PendingCallbacksOrKeyId pending_callbacks =
        std::exchange(pending_callbacks_or_key_id_, key_id);
    for (auto& callback : std::get<PendingCallbacks>(pending_callbacks)) {
      std::move(callback).Run(key_id);
    }
  }

  void RunCallbacksWithFailure(ServiceError error) {
    CHECK(!HasKeyId());
    for (auto& callback : std::exchange(GetCallbacks(), PendingCallbacks())) {
      std::move(callback).Run(base::unexpected(error));
    }
  }

 private:
  PendingCallbacks& GetCallbacks() {
    CHECK(!HasKeyId());
    return std::get<PendingCallbacks>(pending_callbacks_or_key_id_);
  }

  // Holds the value of its first alternative type by default.
  PendingCallbacksOrKeyId pending_callbacks_or_key_id_;
};

}  // namespace

// Templated repository class that manages maps for a single key type.
//
// Responsibilities:
// - Owns and manages the lifetime of loaded unexportable keys of type
//   `RefCountedKeyType`.
// - Maps key IDs to loaded `scoped_refptr` key objects.
// - Maps wrapped keys and tags to their resolved key IDs.
// - Deduplicates concurrent asynchronous load requests for the same wrapped
//   key.
//
// Invariants maintained:
// 1. Bidirectional Consistency: A key ID `id` is present in `key_by_key_id_`
//    if and only if its corresponding wrapped key/tag maps to `id` (or is
//    pending resolution) in `key_id_by_wrapped_key_and_tag_`.
// 2. Load Deduplication: Concurrent calls to `RegisterUnwrapKeyCallback()`
//    for the same wrapped key return the same queue, only returning `1` for
//    the first call to indicate that a new background task should be
//    scheduled.
// 3. Destruction Safety: If a `MaybePendingKeyId` or the repository is
//    destroyed, any queued callbacks are immediately cancelled and notified.
template <typename RefCountedKeyType>
class UnexportableKeyServiceImpl::KeyRepository {
 public:
  using KeyIdType = typename RefCountedKeyType::IdType;
  using KeyIdMap =
      absl::flat_hash_map<KeyIdType, scoped_refptr<RefCountedKeyType>>;

  // Retrieve key operations.
  RefCountedKeyType* GetKey(KeyIdType key_id) const {
    const auto* key = base::FindOrNull(key_by_key_id_, key_id);
    return key ? key->get() : nullptr;
  }

  // Key storage management.
  void Clear() {
    key_by_key_id_.clear();
    key_id_by_wrapped_key_and_tag_.clear();
    weak_ptr_factory_.InvalidateWeakPtrs();
  }

  // Extract key and erase from maps.
  scoped_refptr<RefCountedKeyType> ExtractKey(KeyIdType key_id) {
    auto key_handle = key_by_key_id_.extract(key_id);
    if (!key_handle) {
      return nullptr;
    }
    scoped_refptr<RefCountedKeyType> key = std::move(key_handle.mapped());
    auto wrapped_key_and_tag_handle =
        key_id_by_wrapped_key_and_tag_.extract(GetWrappedKeyAndTag(*key));
    CHECK(wrapped_key_and_tag_handle);
    auto& mapped_key_id = wrapped_key_and_tag_handle.mapped();
    CHECK(mapped_key_id.HasKeyId());
    CHECK_EQ(mapped_key_id.GetKeyId(), key_id);
    return key;
  }

  // Check if repository contains key.
  bool Contains(KeyIdType key_id) const {
    return key_by_key_id_.contains(key_id);
  }

  // Registers `callback` for the key. If the key is already loaded, runs the
  // callback immediately. Otherwise, queues the callback and returns the number
  // of callbacks currently queued for this key (returning 1 if this is the
  // first callback queued, meaning the caller must schedule a load).
  //
  // NOTE: In case the key does not exist in the map yet and we ask the backend
  // for the matching signing key, the application tag returned by the platform
  // must match the tag stored in the config. This invariant is CHECKed in
  // `OnKeyCreatedFromWrappedKey`.
  size_t RegisterUnwrapKeyCallback(
      WrappedKeyAndTagView wrapped_key_and_tag_view,
      base::OnceCallback<void(ServiceErrorOr<KeyIdType>)> callback) {
    auto& [_, maybe_pending_key_id] =
        *key_id_by_wrapped_key_and_tag_.lazy_emplace(
            wrapped_key_and_tag_view, [&](const auto& ctor) {
              ctor(Materialize(wrapped_key_and_tag_view),
                   MaybePendingKeyId<KeyIdType>());
            });

    if (maybe_pending_key_id.HasKeyId()) {
      std::move(callback).Run(maybe_pending_key_id.GetKeyId());
      return 0;
    }

    return maybe_pending_key_id.AddCallback(std::move(callback));
  }

  void OnKeyCreatedFromWrappedKey(
      WrappedKeyAndTag wrapped_key_and_tag,
      ServiceErrorOr<scoped_refptr<RefCountedKeyType>> key_or_error) {
    auto it = key_id_by_wrapped_key_and_tag_.find(wrapped_key_and_tag);
    if (it == key_id_by_wrapped_key_and_tag_.end()) {
      DVLOG(1) << "`wrapped_key` is unknown, did the key get deleted?";
      return;
    }

    MaybePendingKeyId<KeyIdType>& maybe_pending_callbacks = it->second;
    if (maybe_pending_callbacks.HasKeyId()) {
      // If there is already a key ID for this wrapped key, it means that the
      // key id has been resolved in the meantime. In this case, there is
      // nothing to do and we can return immediately.
      return;
    }

    ASSIGN_OR_RETURN(scoped_refptr<RefCountedKeyType> key,
                     std::move(key_or_error), [&](ServiceError error) {
                       auto node = key_id_by_wrapped_key_and_tag_.extract(it);
                       node.mapped().RunCallbacksWithFailure(error);
                     });
    // `key` must be non-null if `key_or_error` holds a value.
    CHECK(key);
    CHECK(wrapped_key_and_tag == GetWrappedKeyAndTag(*key));

    KeyIdType key_id = key->id();
    // A newly created key ID must be unique.
    CHECK(key_by_key_id_.try_emplace(key_id, std::move(key)).second);
    maybe_pending_callbacks.SetKeyIdAndRunCallbacks(key_id);
  }

  ServiceErrorOr<KeyIdType> OnKeyGenerated(
      ServiceErrorOr<scoped_refptr<RefCountedKeyType>> key_or_error) {
    ASSIGN_OR_RETURN(scoped_refptr<RefCountedKeyType> key,
                     std::move(key_or_error));
    // `key` must be non-null if `key_or_error` holds a value.
    CHECK(key);
    KeyIdType key_id = key->id();
    if (!key_id_by_wrapped_key_and_tag_
             .try_emplace(GetWrappedKeyAndTag(*key), key_id)
             .second) {
      // Drop a newly generated key in the case of a key collision. This should
      // be extremely rare.
      DVLOG(1) << "Collision between an existing and a newly generated key "
                  "detected.";
      return base::unexpected(ServiceError::kKeyCollision);
    }
    // A newly generated key ID must be unique.
    CHECK(key_by_key_id_.try_emplace(key_id, std::move(key)).second);
    return key_id;
  }

  base::WeakPtr<KeyRepository> GetWeakPtr() {
    return weak_ptr_factory_.GetWeakPtr();
  }

 private:
  using WrappedKeyAndTagMap = absl::flat_hash_map<WrappedKeyAndTag,
                                                  MaybePendingKeyId<KeyIdType>,
                                                  WrappedKeyAndTagViewHash,
                                                  std::ranges::equal_to>;

  WrappedKeyAndTagMap key_id_by_wrapped_key_and_tag_;
  KeyIdMap key_by_key_id_;
  base::WeakPtrFactory<KeyRepository> weak_ptr_factory_{this};
};

UnexportableKeyServiceImpl::UnexportableKeyServiceImpl(
    UnexportableKeyTaskManager& task_manager,
    BackgroundTaskOrigin task_origin,
    crypto::UnexportableKeyProvider::Config config)
    : task_manager_(task_manager),
      task_origin_(task_origin),
      config_(config),
      signing_keys_(std::make_unique<SigningKeyRepository>()),
      attestation_keys_(std::make_unique<AttestationKeyRepository>()) {}

UnexportableKeyServiceImpl::~UnexportableKeyServiceImpl() = default;

// static
bool UnexportableKeyServiceImpl::IsUnexportableKeyProviderSupported(
    crypto::UnexportableKeyProvider::Config config) {
  return UnexportableKeyTaskManager::GetUnexportableKeyProvider(
             std::move(config)) != nullptr;
}

// static
bool UnexportableKeyServiceImpl::IsStatefulUnexportableKeyProviderSupported(
    crypto::UnexportableKeyProvider::Config config) {
  std::unique_ptr<crypto::UnexportableKeyProvider> provider =
      UnexportableKeyTaskManager::GetUnexportableKeyProvider(std::move(config));
  return provider != nullptr &&
         provider->AsStatefulUnexportableKeyProvider() != nullptr;
}

void UnexportableKeyServiceImpl::GenerateSigningKeySlowlyAsync(
    base::span<const crypto::SignatureVerifier::SignatureAlgorithm>
        acceptable_algorithms,
    BackgroundTaskPriority priority,
    base::OnceCallback<void(ServiceErrorOr<UnexportableSigningKeyId>)>
        callback) {
  // TODO(crbug.com/501056920): Log
  // Crypto.UnexportableKeys.SparePool.Signing.RequestLatency covering both pool
  // hits and fallback misses.
  if (base::FeatureList::IsEnabled(kEnableUnexportableKeysSpareKeyPool)) {
    scoped_refptr<RefCountedUnexportableSigningKey> spare_key =
        PopSpareSigningKey(acceptable_algorithms);
    if (spare_key) {
      // TODO(crbug.com/501056920): Return the key. For now stub returns null.
      NOTIMPLEMENTED();
      return;
    }
  }

  task_manager_->GenerateSigningKeySlowlyAsync(
      task_origin_, config_, acceptable_algorithms, priority,
      WrapCallbackWithErrorIfCancelled(
          std::move(callback),
          // SAFETY: `signing_keys_` is owned by `this` and is guaranteed to be
          // alive if the projection callback is invoked (which only happens if
          // the service is still alive).
          base::BindOnce(&SigningKeyRepository::OnKeyGenerated,
                         base::Unretained(signing_keys_.get()))));
}

void UnexportableKeyServiceImpl::FromWrappedSigningKeySlowlyAsync(
    base::span<const uint8_t> wrapped_key,
    BackgroundTaskPriority priority,
    base::OnceCallback<void(ServiceErrorOr<UnexportableSigningKeyId>)>
        callback) {
  WrappedKeyAndTagView key_view = {wrapped_key, GetApplicationTag(config_)};
  if (signing_keys_->RegisterUnwrapKeyCallback(key_view, std::move(callback)) ==
      1) {
    // NOTE: We don't wrap the callback in `WrapCallbackWithErrorIfCancelled`
    // here, but rather run the callbacks explicitly during the destruction of
    // `MaybePendingKeyId`.
    task_manager_->FromWrappedSigningKeySlowlyAsync(
        task_origin_, config_, wrapped_key, priority,
        base::BindOnce(&SigningKeyRepository::OnKeyCreatedFromWrappedKey,
                       signing_keys_->GetWeakPtr(), Materialize(key_view)));
  }
}

void UnexportableKeyServiceImpl::GenerateAttestationKeySlowlyAsync(
    base::span<const crypto::SignatureVerifier::SignatureAlgorithm>
        acceptable_algorithms,
    BackgroundTaskPriority priority,
    base::OnceCallback<void(ServiceErrorOr<UnexportableAttestationKeyId>)>
        callback) {
  task_manager_->GenerateAttestationKeySlowlyAsync(
      task_origin_, config_, acceptable_algorithms, priority,
      WrapCallbackWithErrorIfCancelled(
          std::move(callback),
          // SAFETY: `attestation_keys_` is owned by `this` and is guaranteed to
          // be alive if the projection callback is invoked (which only happens
          // if the service is still alive).
          base::BindOnce(&AttestationKeyRepository::OnKeyGenerated,
                         base::Unretained(attestation_keys_.get()))));
}

void UnexportableKeyServiceImpl::FromWrappedAttestationKeySlowlyAsync(
    base::span<const uint8_t> wrapped_key,
    BackgroundTaskPriority priority,
    base::OnceCallback<void(ServiceErrorOr<UnexportableAttestationKeyId>)>
        callback) {
  WrappedKeyAndTagView key_view = {wrapped_key, GetApplicationTag(config_)};
  if (attestation_keys_->RegisterUnwrapKeyCallback(key_view,
                                                   std::move(callback)) == 1) {
    // NOTE: We don't wrap the callback in `WrapCallbackWithErrorIfCancelled`
    // here, but rather run the callbacks explicitly during the destruction of
    // `MaybePendingKeyId`.
    task_manager_->FromWrappedAttestationKeySlowlyAsync(
        task_origin_, config_, wrapped_key, priority,
        base::BindOnce(&AttestationKeyRepository::OnKeyCreatedFromWrappedKey,
                       attestation_keys_->GetWeakPtr(), Materialize(key_view)));
  }
}

void UnexportableKeyServiceImpl::GetAllKeysForGarbageCollectionSlowlyAsync(
    BackgroundTaskPriority priority,
    base::OnceCallback<void(ServiceErrorOr<std::vector<UnexportableKeyId>>)>
        callback) {
  task_manager_->GetAllKeysForGarbageCollectionSlowlyAsync(
      task_origin_, config_, priority,
      WrapCallbackWithErrorIfCancelled(
          std::move(callback),
          // SAFETY: `this` is guaranteed to be alive if the projection callback
          // is invoked.
          base::BindOnce(&UnexportableKeyServiceImpl::
                             OnGetAllKeysForGarbageCollectionSlowlyImpl,
                         base::Unretained(this))));
}

void UnexportableKeyServiceImpl::SignSlowlyAsync(
    UnexportableSigningKeyId key_id,
    base::span<const uint8_t> data,
    BackgroundTaskPriority priority,
    base::OnceCallback<void(ServiceErrorOr<std::vector<uint8_t>>)> callback) {
  if (auto* key = signing_keys_->GetKey(key_id)) {
    task_manager_->SignSlowlyAsync(
        task_origin_, base::WrapRefCounted(key), data, priority,
        WrapCallbackWithErrorIfCancelled(std::move(callback)));
    return;
  }

  std::move(callback).Run(base::unexpected(ServiceError::kKeyNotFound));
}

void UnexportableKeyServiceImpl::CertifySlowlyAsync(
    UnexportableAttestationKeyId attestation_key_id,
    UnexportableSigningKeyId signing_key_id,
    base::span<const uint8_t> challenge,
    BackgroundTaskPriority priority,
    base::OnceCallback<void(ServiceErrorOr<crypto::AttestationStatement>)>
        callback) {
  auto* attestation_key = attestation_keys_->GetKey(attestation_key_id);
  if (!attestation_key) {
    std::move(callback).Run(base::unexpected(ServiceError::kKeyNotFound));
    return;
  }

  auto* signing_key = signing_keys_->GetKey(signing_key_id);
  if (!signing_key) {
    std::move(callback).Run(base::unexpected(ServiceError::kKeyNotFound));
    return;
  }

  task_manager_->CertifySlowlyAsync(
      task_origin_, base::WrapRefCounted(attestation_key),
      base::WrapRefCounted(signing_key), challenge, priority,
      WrapCallbackWithErrorIfCancelled(std::move(callback)));
}

void UnexportableKeyServiceImpl::DeleteKeysSlowlyAsync(
    base::span<const UnexportableKeyId> key_ids,
    BackgroundTaskPriority priority,
    base::OnceCallback<void(ServiceErrorOr<size_t>)> callback) {
  // Delete the keys from the in-memory maps.
  std::vector<ServiceErrorOr<scoped_refptr<RefCountedUnexportableKey>>>
      keys_or_errors = base::ToVector(key_ids, [&](UnexportableKeyId key_id) {
        return ExtractKeyFromMaps(key_id);
      });

  // Collect the keys that were successfully deleted.
  std::erase_if(keys_or_errors, [](auto& k) { return !k.has_value(); });
  std::vector<scoped_refptr<RefCountedUnexportableKey>> keys_to_delete =
      base::ToVector(keys_or_errors, [](auto& key) { return *std::move(key); });

  // If no keys were deleted, return an error.
  if (keys_to_delete.empty()) {
    std::move(callback).Run(base::unexpected(ServiceError::kKeyNotFound));
    return;
  }

  task_manager_->DeleteKeysSlowlyAsync(
      task_origin_, config_, std::move(keys_to_delete), priority,
      WrapCallbackWithErrorIfCancelled(std::move(callback)));
}

void UnexportableKeyServiceImpl::DeleteAllKeysSlowlyAsync(
    base::OnceCallback<void(ServiceErrorOr<size_t>)> callback) {
  signing_keys_->Clear();
  attestation_keys_->Clear();
  all_gc_keys_by_key_id_.clear();

  // Invalidate weak pointers to cancel pending key lookup requests.
  weak_ptr_factory_.InvalidateWeakPtrs();

  task_manager_->DeleteAllKeysSlowlyAsync(
      task_origin_, config_, BackgroundTaskPriority::kUserBlocking,
      WrapCallbackWithErrorIfCancelled(std::move(callback)));
}

ServiceErrorOr<std::vector<uint8_t>>
UnexportableKeyServiceImpl::GetSubjectPublicKeyInfo(
    UnexportableKeyId key_id) const {
  ASSIGN_OR_RETURN(const crypto::UnexportableKey* key, GetKey(key_id));
  return key->GetSubjectPublicKeyInfo();
}

ServiceErrorOr<std::vector<uint8_t>> UnexportableKeyServiceImpl::GetWrappedKey(
    UnexportableKeyId key_id) const {
  ASSIGN_OR_RETURN(const crypto::UnexportableKey* key, GetKey(key_id));
  return key->GetWrappedKey();
}

ServiceErrorOr<crypto::SignatureVerifier::SignatureAlgorithm>
UnexportableKeyServiceImpl::GetAlgorithm(UnexportableKeyId key_id) const {
  ASSIGN_OR_RETURN(const crypto::UnexportableKey* key, GetKey(key_id));
  return key->Algorithm();
}

ServiceErrorOr<std::string> UnexportableKeyServiceImpl::GetKeyTag(
    UnexportableKeyId key_id) const {
  ASSIGN_OR_RETURN(const crypto::StatefulKey* stateful_key,
                   GetStatefulKey(key_id));
  return stateful_key->GetKeyTag();
}

ServiceErrorOr<base::Time> UnexportableKeyServiceImpl::GetCreationTime(
    UnexportableKeyId key_id) const {
  ASSIGN_OR_RETURN(const crypto::StatefulKey* stateful_key,
                   GetStatefulKey(key_id));
  return stateful_key->GetCreationTime();
}

ServiceErrorOr<const crypto::UnexportableKey*>
UnexportableKeyServiceImpl::GetKey(UnexportableKeyId key_id) const {
  if (auto* key = signing_keys_->GetKey(UnexportableSigningKeyId(key_id))) {
    return &key->key();
  }
  if (auto* key =
          attestation_keys_->GetKey(UnexportableAttestationKeyId(key_id))) {
    return &key->key();
  }
  if (const auto* key = base::FindOrNull(all_gc_keys_by_key_id_, key_id)) {
    return &(*key)->key();
  }
  return base::unexpected(ServiceError::kKeyNotFound);
}

ServiceErrorOr<const crypto::StatefulKey*>
UnexportableKeyServiceImpl::GetStatefulKey(UnexportableKeyId key_id) const {
  ASSIGN_OR_RETURN(const crypto::UnexportableKey* key, GetKey(key_id));
  if (const crypto::StatefulKey* stateful_key = key->AsStatefulKey()) {
    return stateful_key;
  }
  return base::unexpected(ServiceError::kOperationNotSupported);
}

ServiceErrorOr<scoped_refptr<RefCountedUnexportableKey>>
UnexportableKeyServiceImpl::ExtractKeyFromMaps(UnexportableKeyId key_id) {
  // Check the garbage collection map first. Ensure the `key_id` can't be
  // present in the other maps.
  if (auto gc_key_handle = all_gc_keys_by_key_id_.extract(key_id)) {
    CHECK(!signing_keys_->Contains(UnexportableSigningKeyId(key_id)));
    CHECK(!attestation_keys_->Contains(UnexportableAttestationKeyId(key_id)));
    return std::move(gc_key_handle.mapped());
  }

  if (auto key = signing_keys_->ExtractKey(UnexportableSigningKeyId(key_id))) {
    return key;
  }

  if (auto key =
          attestation_keys_->ExtractKey(UnexportableAttestationKeyId(key_id))) {
    return key;
  }

  return base::unexpected(ServiceError::kKeyNotFound);
}

ServiceErrorOr<std::vector<UnexportableKeyId>>
UnexportableKeyServiceImpl::OnGetAllKeysForGarbageCollectionSlowlyImpl(
    ServiceErrorOr<std::vector<scoped_refptr<RefCountedUnexportableKey>>>
        keys_or_error) {
  ASSIGN_OR_RETURN(std::vector<scoped_refptr<RefCountedUnexportableKey>> keys,
                   std::move(keys_or_error));

  auto key_ids = base::ToVector(keys, [](auto& key) { return key->id(); });
  all_gc_keys_by_key_id_.clear();
  all_gc_keys_by_key_id_.reserve(keys.size());
  for (auto& key : keys) {
    all_gc_keys_by_key_id_.emplace(key->id(), std::move(key));
  }
  return key_ids;
}

scoped_refptr<RefCountedUnexportableSigningKey>
UnexportableKeyServiceImpl::PopSpareSigningKey(
    base::span<const crypto::SignatureVerifier::SignatureAlgorithm>
        acceptable_algorithms) {
  NOTIMPLEMENTED();
  // TODO(crbug.com/501056920): Implement this to pop a key from
  // `spare_signing_keys_pool_`.
  // TODO(crbug.com/501056920): Implement the following:
  // - Log Crypto.UnexportableKeys.SparePool.Signing.RetrievalResult
  // - Log Crypto.UnexportableKeys.SparePool.Signing.PoolSize
  return nullptr;
}

void UnexportableKeyServiceImpl::ReplenishSpareSigningKeyPoolAsync(
    base::span<const crypto::SignatureVerifier::SignatureAlgorithm>
        acceptable_algorithms) {
  NOTIMPLEMENTED();
  // TODO(crbug.com/501056920): Implement this to start background key
  // generation using a lambda for the callback instead of a separate method.
}

}  // namespace unexportable_keys
