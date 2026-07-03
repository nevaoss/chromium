// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/themes/ntp_custom_background_service_base.h"

#include "base/files/file_path.h"
#include "components/prefs/pref_service.h"
#include "components/themes/ntp_background_service.h"
#include "components/themes/ntp_custom_background_service_constants.h"

// static
base::DictValue NtpCustomBackgroundServiceBase::NtpCustomBackgroundDefaults() {
  base::DictValue defaults;
  defaults.Set(kNtpCustomBackgroundURL, base::Value(base::Value::Type::STRING));
  defaults.Set(kNtpCustomBackgroundAttributionLine1,
               base::Value(base::Value::Type::STRING));
  defaults.Set(kNtpCustomBackgroundAttributionLine2,
               base::Value(base::Value::Type::STRING));
  defaults.Set(kNtpCustomBackgroundAttributionActionURL,
               base::Value(base::Value::Type::STRING));
  defaults.Set(kNtpCustomBackgroundCollectionId,
               base::Value(base::Value::Type::STRING));
  defaults.Set(kNtpCustomBackgroundResumeToken,
               base::Value(base::Value::Type::STRING));
  defaults.Set(kNtpCustomBackgroundRefreshTimestamp,
               base::Value(base::Value::Type::INTEGER));
  return defaults;
}

// static
base::DictValue NtpCustomBackgroundServiceBase::GetBackgroundInfoAsDict(
    const GURL& background_url,
    const std::string& attribution_line_1,
    const std::string& attribution_line_2,
    const GURL& action_url,
    const std::optional<std::string>& collection_id,
    const std::optional<std::string>& resume_token,
    std::optional<int> refresh_timestamp) {
  base::DictValue background_info;
  background_info.Set(kNtpCustomBackgroundURL,
                      base::Value(background_url.spec()));
  background_info.Set(kNtpCustomBackgroundAttributionLine1,
                      base::Value(attribution_line_1));
  background_info.Set(kNtpCustomBackgroundAttributionLine2,
                      base::Value(attribution_line_2));
  background_info.Set(kNtpCustomBackgroundAttributionActionURL,
                      base::Value(action_url.spec()));
  background_info.Set(kNtpCustomBackgroundCollectionId,
                      base::Value(collection_id.value_or("")));
  background_info.Set(kNtpCustomBackgroundResumeToken,
                      base::Value(resume_token.value_or("")));
  background_info.Set(kNtpCustomBackgroundRefreshTimestamp,
                      base::Value(refresh_timestamp.value_or(0)));
  return background_info;
}

NtpCustomBackgroundServiceBase::NtpCustomBackgroundServiceBase(
    PrefService* pref_service,
    NtpBackgroundService* background_service,
    const std::string& custom_background_dict_pref_name,
    const std::string& local_to_device_pref_name)
    : pref_service_(pref_service),
      background_service_(background_service),
      custom_background_dict_pref_name_(custom_background_dict_pref_name),
      local_to_device_pref_name_(local_to_device_pref_name) {}

NtpCustomBackgroundServiceBase::~NtpCustomBackgroundServiceBase() {
  for (auto& observer : observers_) {
    observer.OnNtpCustomBackgroundServiceShuttingDown();
  }
}

void NtpCustomBackgroundServiceBase::OnCollectionInfoAvailable() {}
void NtpCustomBackgroundServiceBase::OnCollectionImagesAvailable() {}
void NtpCustomBackgroundServiceBase::OnNextCollectionImageAvailable() {}
void NtpCustomBackgroundServiceBase::OnNtpBackgroundServiceShuttingDown() {}

void NtpCustomBackgroundServiceBase::AddObserver(
    NtpCustomBackgroundServiceObserver* observer) {
  observers_.AddObserver(observer);
}
void NtpCustomBackgroundServiceBase::RemoveObserver(
    NtpCustomBackgroundServiceObserver* observer) {
  observers_.RemoveObserver(observer);
}
void NtpCustomBackgroundServiceBase::NotifyAboutBackgrounds() {
  for (NtpCustomBackgroundServiceObserver& observer : observers_) {
    observer.OnCustomBackgroundImageUpdated();
  }
}

void NtpCustomBackgroundServiceBase::ResetCustomBackgroundInfo() {
  SetCustomBackgroundInfo(GURL(), GURL(), std::string(), std::string(), GURL(),
                          std::string());
}

void NtpCustomBackgroundServiceBase::SetCustomBackgroundInfo(
    const GURL& background_url,
    const GURL& thumbnail_url,
    const std::string& attribution_line_1,
    const std::string& attribution_line_2,
    const GURL& action_url,
    const std::string& collection_id) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  bool is_backdrop_collection =
      background_service_ &&
      background_service_->IsValidBackdropCollection(collection_id);
  bool is_backdrop_url =
      background_service_ &&
      background_service_->IsValidBackdropUrl(background_url);

  bool need_forced_refresh =
      pref_service_->GetBoolean(local_to_device_pref_name_) &&
      pref_service_->FindPreference(custom_background_dict_pref_name_)
          ->IsDefaultValue();

  pref_service_->SetBoolean(local_to_device_pref_name_, false);

  if (!background_url.is_valid() && !collection_id.empty() &&
      is_backdrop_collection) {
    background_service_->FetchNextCollectionImage(collection_id, std::nullopt);
  } else if (background_url.is_valid() && is_backdrop_url) {
    base::DictValue background_info = GetBackgroundInfoAsDict(
        background_url, attribution_line_1, attribution_line_2, action_url,
        collection_id, std::nullopt, std::nullopt);
    pref_service_->SetDict(custom_background_dict_pref_name_,
                           std::move(background_info));
  } else {
    pref_service_->ClearPref(custom_background_dict_pref_name_);

    // If this device was using a local image and did not have a non-local
    // background saved, UpdateBackgroundFromSync will not fire. Therefore, we
    // need to force a refresh here.
    if (need_forced_refresh) {
      NotifyAboutBackgrounds();
    }
  }
}


bool NtpCustomBackgroundServiceBase::IsCustomBackgroundPrefValid() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  const base::DictValue& background_info =
      pref_service_->GetDict(custom_background_dict_pref_name_);

  const base::Value* background_url =
      background_info.Find(kNtpCustomBackgroundURL);
  if (!background_url) {
    return false;
  }

  return GURL(background_url->GetString()).is_valid();
}
