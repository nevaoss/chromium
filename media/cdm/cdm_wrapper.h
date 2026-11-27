// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_CDM_CDM_WRAPPER_H_
#define MEDIA_CDM_CDM_WRAPPER_H_

#include <stdint.h>

#include "base/check.h"
#include "base/memory/raw_ptr.h"
#include "media/base/cdm_config.h"
#include "media/base/media_switches.h"
#if defined(USE_NEVA_CDM)
#include "media/cdm/neva/webos/content_decryption_module_webos.h"
#endif
#include "media/cdm/api/content_decryption_module.h"
#include "media/cdm/cdm_helpers.h"
#include "media/cdm/supported_cdm_versions.h"

namespace media {

namespace {

#if defined(USE_NEVA_CDM)
bool IsEncryptionSchemeSupportedByLegacyCdms(
    const cdm::EncryptionScheme& scheme) {
  // CDM_9 don't check the encryption scheme, so do it here.
  return scheme == cdm::EncryptionScheme::kUnencrypted ||
         scheme == cdm::EncryptionScheme::kCenc;
}

cdm::AudioDecoderConfig ToAudioDecoderConfig(
    const cdm::AudioDecoderConfig_2& config) {
  return {config.codec,
          config.channel_count,
          config.bits_per_channel,
          config.samples_per_second,
          config.extra_data,
          config.extra_data_size};
}

cdm::VideoDecoderConfig ToVideoDecoderConfig(
    const cdm::VideoDecoderConfig_2& config) {
  return {config.codec,      config.profile,    config.format,
          config.coded_size, config.extra_data, config.extra_data_size};
}

cdm::InputBuffer ToInputBuffer(const cdm::InputBuffer_2& buffer,
                               const cdm::StreamType decoder_type) {
  return {buffer.data,
          buffer.data_size,
          buffer.encryption_scheme,
          buffer.key_id,
          buffer.key_id_size,
          buffer.iv,
          buffer.iv_size,
          buffer.subsamples,
          buffer.num_subsamples,
          buffer.timestamp,
          buffer.pattern,
          decoder_type ==
              static_cast<uint32_t>(cdm::StreamType::kStreamTypeVideo ? 1 : 0)};
}
#endif

cdm::VideoDecoderConfig_2 ToVideoDecoderConfig_2(
    const cdm::VideoDecoderConfig_3& config) {
  return {config.codec,
          config.profile,
          config.format,
          config.coded_size,
          config.extra_data,
          config.extra_data_size,
          config.encryption_scheme};
}

}  // namespace

// Returns a pointer to the requested CDM upon success.
// Returns NULL if an error occurs or the requested |cdm_interface_version| or
// |key_system| is not supported or another error occurs.
// The caller should cast the returned pointer to the type matching
// |cdm_interface_version|.
// Caller retains ownership of arguments and must call Destroy() on the returned
// object.
typedef void* (*CreateCdmFunc)(int cdm_interface_version,
                               const char* key_system,
                               uint32_t key_system_size,
                               GetCdmHostFunc get_cdm_host_func,
                               void* user_data);

// CdmWrapper wraps different versions of ContentDecryptionModule interfaces and
// exposes a common interface to the caller.
//
// The caller should call CdmWrapper::Create() to create a CDM instance.
// CdmWrapper will first try to create a CDM instance that supports the latest
// CDM interface (ContentDecryptionModule). If such an instance cannot be
// created (e.g. an older CDM was loaded), CdmWrapper will try to create a CDM
// that supports an older version of CDM interface (e.g.
// ContentDecryptionModule_*). Internally CdmWrapper converts the CdmWrapper
// calls to corresponding ContentDecryptionModule calls.
//
// Since this file is highly templated and default implementations are short
// (just a shim layer in most cases), everything is done in this header file.
//
class CdmWrapper {
 public:
  static CdmWrapper* Create(CreateCdmFunc create_cdm_func,
                            const CdmConfig& cdm_config,
                            GetCdmHostFunc get_cdm_host_func,
                            void* user_data);

  CdmWrapper(const CdmWrapper&) = delete;
  CdmWrapper& operator=(const CdmWrapper&) = delete;

  virtual ~CdmWrapper() {}

  // Returns the version of the CDM interface that the created CDM uses.
  virtual int GetInterfaceVersion() = 0;

  // Initializes the CDM instance and returns whether OnInitialized() will be
  // called on the host. The caller should NOT wait for OnInitialized() if false
  // is returned.
  virtual bool Initialize(const CdmConfig& cdm_config) = 0;

  virtual void SetServerCertificate(uint32_t promise_id,
                                    const uint8_t* server_certificate_data,
                                    uint32_t server_certificate_data_size) = 0;

  // Gets the key status for a policy that contains the |min_hdcp_version|.
  // Returns whether GetStatusForPolicy() is supported. If true, the CDM should
  // resolve or reject the promise. If false, the caller will reject the
  // promise.
  [[nodiscard]] virtual bool GetStatusForPolicy(
      uint32_t promise_id,
      cdm::HdcpVersion min_hdcp_version) = 0;

  virtual void CreateSessionAndGenerateRequest(uint32_t promise_id,
                                               cdm::SessionType session_type,
                                               cdm::InitDataType init_data_type,
                                               const uint8_t* init_data,
                                               uint32_t init_data_size) = 0;
  virtual void LoadSession(uint32_t promise_id,
                           cdm::SessionType session_type,
                           const char* session_id,
                           uint32_t session_id_size) = 0;
  virtual void UpdateSession(uint32_t promise_id,
                             const char* session_id,
                             uint32_t session_id_size,
                             const uint8_t* response,
                             uint32_t response_size) = 0;
  virtual void CloseSession(uint32_t promise_id,
                            const char* session_id,
                            uint32_t session_id_size) = 0;
  virtual void RemoveSession(uint32_t promise_id,
                             const char* session_id,
                             uint32_t session_id_size) = 0;
  virtual void TimerExpired(void* context) = 0;
  virtual cdm::Status Decrypt(const cdm::InputBuffer_2& encrypted_buffer,
#if defined(USE_NEVA_CDM)
                              cdm::DecryptedBlock* decrypted_buffer,
                              const cdm::StreamType decoder_type) = 0;
#else
                              cdm::DecryptedBlock* decrypted_buffer) = 0;
#endif
  virtual cdm::Status InitializeAudioDecoder(
      const cdm::AudioDecoderConfig_2& audio_decoder_config) = 0;
  virtual cdm::Status InitializeVideoDecoder(
      const cdm::VideoDecoderConfig_3& video_decoder_config) = 0;
  virtual void DeinitializeDecoder(cdm::StreamType decoder_type) = 0;
  virtual void ResetDecoder(cdm::StreamType decoder_type) = 0;
  virtual cdm::Status DecryptAndDecodeFrame(
      const cdm::InputBuffer_2& encrypted_buffer,
      media::VideoFrameImpl* video_frame) = 0;
  virtual cdm::Status DecryptAndDecodeSamples(
      const cdm::InputBuffer_2& encrypted_buffer,
      cdm::AudioFrames* audio_frames) = 0;
  virtual void OnPlatformChallengeResponse(
      const cdm::PlatformChallengeResponse& response) = 0;
  virtual void OnQueryOutputProtectionStatus(
      cdm::QueryResult result,
      uint32_t link_mask,
      uint32_t output_protection_mask) = 0;
  virtual void OnStorageId(uint32_t version,
                           const uint8_t* storage_id,
                           uint32_t storage_id_size) = 0;

 protected:
  CdmWrapper() {}
};

// Template class that does the CdmWrapper -> CdmInterface conversion. Default
// implementations are provided. Any methods that need special treatment should
// be specialized.
template <int CdmInterfaceVersion>
class CdmWrapperImpl : public CdmWrapper {
 public:
  using CdmInterface =
      typename CdmInterfaceTraits<CdmInterfaceVersion>::CdmInterface;
  static_assert(CdmInterfaceVersion == CdmInterface::kVersion,
                "CDM interface version mismatch.");

  static CdmWrapper* Create(CreateCdmFunc create_cdm_func,
                            const CdmConfig& cdm_config,
                            GetCdmHostFunc get_cdm_host_func,
                            void* user_data) {
    void* cdm_instance = create_cdm_func(
        CdmInterfaceVersion, cdm_config.key_system.data(),
        base::checked_cast<uint32_t>(cdm_config.key_system.size()),
        get_cdm_host_func, user_data);
    if (!cdm_instance)
      return nullptr;

    return new CdmWrapperImpl<CdmInterfaceVersion>(
        static_cast<CdmInterface*>(cdm_instance));
  }

  CdmWrapperImpl(const CdmWrapperImpl&) = delete;
  CdmWrapperImpl& operator=(const CdmWrapperImpl&) = delete;

  ~CdmWrapperImpl() override = default;

  int GetInterfaceVersion() override { return CdmInterfaceVersion; }

  bool Initialize(const CdmConfig& cdm_config) override {
    cdm_->Initialize(cdm_config.allow_distinctive_identifier,
                     cdm_config.allow_persistent_state,
                     cdm_config.use_hw_secure_codecs);
    return true;
  }

  void SetServerCertificate(uint32_t promise_id,
                            const uint8_t* server_certificate_data,
                            uint32_t server_certificate_data_size) override {
    cdm_->SetServerCertificate(promise_id, server_certificate_data,
                               server_certificate_data_size);
  }

  bool GetStatusForPolicy(uint32_t promise_id,
                          cdm::HdcpVersion min_hdcp_version) override {
    cdm_->GetStatusForPolicy(promise_id, {min_hdcp_version});
    return true;
  }

  void CreateSessionAndGenerateRequest(uint32_t promise_id,
                                       cdm::SessionType session_type,
                                       cdm::InitDataType init_data_type,
                                       const uint8_t* init_data,
                                       uint32_t init_data_size) override {
    cdm_->CreateSessionAndGenerateRequest(
        promise_id, session_type, init_data_type, init_data, init_data_size);
  }

  void LoadSession(uint32_t promise_id,
                   cdm::SessionType session_type,
                   const char* session_id,
                   uint32_t session_id_size) override {
    cdm_->LoadSession(promise_id, session_type, session_id, session_id_size);
  }

  void UpdateSession(uint32_t promise_id,
                     const char* session_id,
                     uint32_t session_id_size,
                     const uint8_t* response,
                     uint32_t response_size) override {
    cdm_->UpdateSession(promise_id, session_id, session_id_size, response,
                        response_size);
  }

  void CloseSession(uint32_t promise_id,
                    const char* session_id,
                    uint32_t session_id_size) override {
    cdm_->CloseSession(promise_id, session_id, session_id_size);
  }

  void RemoveSession(uint32_t promise_id,
                     const char* session_id,
                     uint32_t session_id_size) override {
    cdm_->RemoveSession(promise_id, session_id, session_id_size);
  }

  void TimerExpired(void* context) override { cdm_->TimerExpired(context); }

  cdm::Status Decrypt(const cdm::InputBuffer_2& encrypted_buffer,
#if defined(USE_NEVA_CDM)
                      cdm::DecryptedBlock* decrypted_buffer,
                      const cdm::StreamType decoder_type) override {
#else
                      cdm::DecryptedBlock* decrypted_buffer) override {
#endif
    return cdm_->Decrypt(encrypted_buffer, decrypted_buffer);
  }

  cdm::Status InitializeAudioDecoder(
      const cdm::AudioDecoderConfig_2& audio_decoder_config) override {
    return cdm_->InitializeAudioDecoder(audio_decoder_config);
  }

  cdm::Status InitializeVideoDecoder(
      const cdm::VideoDecoderConfig_3& video_decoder_config) override {
    return cdm_->InitializeVideoDecoder(video_decoder_config);
  }

  void DeinitializeDecoder(cdm::StreamType decoder_type) override {
    cdm_->DeinitializeDecoder(decoder_type);
  }

  void ResetDecoder(cdm::StreamType decoder_type) override {
    cdm_->ResetDecoder(decoder_type);
  }

  cdm::Status DecryptAndDecodeFrame(
      const cdm::InputBuffer_2& encrypted_buffer,
      media::VideoFrameImpl* video_frame) override {
    return cdm_->DecryptAndDecodeFrame(encrypted_buffer, video_frame);
  }

  cdm::Status DecryptAndDecodeSamples(
      const cdm::InputBuffer_2& encrypted_buffer,
      cdm::AudioFrames* audio_frames) override {
    return cdm_->DecryptAndDecodeSamples(encrypted_buffer, audio_frames);
  }

  void OnPlatformChallengeResponse(
      const cdm::PlatformChallengeResponse& response) override {
    cdm_->OnPlatformChallengeResponse(response);
  }

  void OnQueryOutputProtectionStatus(cdm::QueryResult result,
                                     uint32_t link_mask,
                                     uint32_t output_protection_mask) override {
    cdm_->OnQueryOutputProtectionStatus(result, link_mask,
                                        output_protection_mask);
  }

  void OnStorageId(uint32_t version,
                   const uint8_t* storage_id,
                   uint32_t storage_id_size) override {
    cdm_->OnStorageId(version, storage_id, storage_id_size);
  }

 private:
  // Used to clean up a CDM instance owned by a std::unique_ptr.
  struct CdmDeleter {
    void operator()(CdmInterface* cdm) const { cdm->Destroy(); }
  };

  explicit CdmWrapperImpl(CdmInterface* cdm) : cdm_(cdm) { CHECK(cdm_); }

  std::unique_ptr<CdmInterface, CdmDeleter> cdm_;
};

#if defined(USE_NEVA_CDM)
template <>
bool CdmWrapperImpl<8>::Initialize(bool allow_distinctive_identifier,
                                   bool allow_persistent_state,
                                   bool /* use_hw_secure_codecs*/) {
  cdm_->Initialize(allow_distinctive_identifier, allow_persistent_state);
  return false;
}

template <>
bool CdmWrapperImpl<8>::GetStatusForPolicy(uint32_t promise_id,
                                           cdm::HdcpVersion min_hdcp_version) {
  return false;
}

template <>
cdm::Status CdmWrapperImpl<8>::InitializeAudioDecoder(
    const cdm::AudioDecoderConfig_2& audio_decoder_config) {
  if (!IsEncryptionSchemeSupportedByLegacyCdms(
          audio_decoder_config.encryption_scheme))
    return cdm::kInitializationError;

  return cdm_->InitializeAudioDecoder(
      ToAudioDecoderConfig(audio_decoder_config));
}

template <>
cdm::Status CdmWrapperImpl<8>::InitializeVideoDecoder(
    const cdm::VideoDecoderConfig_3& video_decoder_config) {
  if (!IsEncryptionSchemeSupportedByLegacyCdms(
          video_decoder_config.encryption_scheme))
    return cdm::kInitializationError;

  return cdm_->InitializeVideoDecoder(
      ToVideoDecoderConfig(ToVideoDecoderConfig_2(video_decoder_config)));
}

template <>
cdm::Status CdmWrapperImpl<8>::Decrypt(
    const cdm::InputBuffer_2& encrypted_buffer,
    cdm::DecryptedBlock* decrypted_buffer,
    const cdm::StreamType decoder_type) {
  if (!IsEncryptionSchemeSupportedByLegacyCdms(
          encrypted_buffer.encryption_scheme))
    return cdm::kDecryptError;

  return cdm_->Decrypt(ToInputBuffer(encrypted_buffer, decoder_type),
                       decrypted_buffer);
}

template <>
cdm::Status CdmWrapperImpl<8>::DecryptAndDecodeFrame(
    const cdm::InputBuffer_2& encrypted_buffer,
    media::VideoFrameImpl* video_frame) {
  if (!IsEncryptionSchemeSupportedByLegacyCdms(
          encrypted_buffer.encryption_scheme))
    return cdm::kDecryptError;

  return cdm_->DecryptAndDecodeFrame(
      ToInputBuffer(encrypted_buffer, cdm::StreamType::kStreamTypeVideo),
      video_frame);
}

template <>
cdm::Status CdmWrapperImpl<8>::DecryptAndDecodeSamples(
    const cdm::InputBuffer_2& encrypted_buffer,
    cdm::AudioFrames* audio_frames) {
  if (!IsEncryptionSchemeSupportedByLegacyCdms(
          encrypted_buffer.encryption_scheme))
    return cdm::kDecryptError;

  return cdm_->DecryptAndDecodeSamples(
      ToInputBuffer(encrypted_buffer, cdm::StreamType::kStreamTypeAudio),
      audio_frames);
}

template <>
void CdmWrapperImpl<8>::OnStorageId(uint32_t version,
                                    const uint8_t* storage_id,
                                    uint32_t storage_id_size) {
  // Nothing to do
}

#endif  // defined(USE_NEVA_CDM)

// Specialization for cdm::ContentDecryptionModule_10 methods and
// cdm::ContentDecryptionModule_11 methods.

template <>
cdm::Status CdmWrapperImpl<10>::InitializeVideoDecoder(
    const cdm::VideoDecoderConfig_3& video_decoder_config) {
  return cdm_->InitializeVideoDecoder(
      ToVideoDecoderConfig_2(video_decoder_config));
}

#if defined(USE_NEVA_CDM)
template <>
cdm::Status CdmWrapperImpl<10>::Decrypt(
    const cdm::InputBuffer_2& encrypted_buffer,
    cdm::DecryptedBlock* decrypted_buffer,
    const cdm::StreamType decoder_type) {
  return cdm_->Decrypt(encrypted_buffer, decrypted_buffer, decoder_type);
}
#endif  // defined(USE_NEVA_CDM)

template <>
cdm::Status CdmWrapperImpl<11>::InitializeVideoDecoder(
    const cdm::VideoDecoderConfig_3& video_decoder_config) {
  return cdm_->InitializeVideoDecoder(
      ToVideoDecoderConfig_2(video_decoder_config));
}

// Method to try creating CdmWrapper recursively iterating through the listed
// versions.
template <int FirstVersion, int... RemainingVersions>
CdmWrapper* CreateWithVersions(CreateCdmFunc create_cdm_func,
                               const CdmConfig& cdm_config,
                               GetCdmHostFunc get_cdm_host_func,
                               void* user_data) {
  if (IsSupportedAndEnabledCdmInterfaceVersion(FirstVersion)) {
    CdmWrapper* wrapper = CdmWrapperImpl<FirstVersion>::Create(
        create_cdm_func, cdm_config, get_cdm_host_func, user_data);
    if (wrapper) {
      return wrapper;
    }
  }

  if constexpr (sizeof...(RemainingVersions) > 0) {
    return CreateWithVersions<RemainingVersions...>(
        create_cdm_func, cdm_config, get_cdm_host_func, user_data);
  } else {
    // Base case for the last version not succeeding.
    return nullptr;
  }
}

// static
CdmWrapper* CdmWrapper::Create(CreateCdmFunc create_cdm_func,
                               const CdmConfig& cdm_config,
                               GetCdmHostFunc get_cdm_host_func,
                               void* user_data) {
#if defined(USE_NEVA_CDM)
  // TODO(sync-to-77) needs to be fixed
  static_assert(CheckSupportedCdmInterfaceVersions(8, 8),
#else
  static_assert(CheckSupportedCdmInterfaceVersions(10, 12),
#endif
                "Mismatch between CdmWrapper::Create() and "
                "IsSupportedCdmInterfaceVersion()");

#if defined(USE_NEVA_CDM)
  return CreateWithVersions<12, 11, 10, 8>(create_cdm_func, cdm_config,
                                           get_cdm_host_func, user_data);
#else   // defined(USE_NEVA_CDM)
  return CreateWithVersions<12, 11, 10>(create_cdm_func, cdm_config,
                                        get_cdm_host_func, user_data);
#endif  // !defined(USE_NEVA_CDM)
}

}  // namespace media

#endif  // MEDIA_CDM_CDM_WRAPPER_H_
