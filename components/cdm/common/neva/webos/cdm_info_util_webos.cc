// Copyright 2019 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include "components/cdm/common/neva/cdm_info_util.h"

#include "base/containers/flat_set.h"
#include "base/files/file_util.h"
#include "base/path_service.h"
#include "base/version.h"
#include "content/public/common/cdm_info.h"
#include "media/base/decrypt_config.h"
#include "media/base/video_codecs.h"

#include "widevine_cdm_version.h"  // In SHARED_INTERMEDIATE_DIR.

namespace {
using Robustness = content::CdmInfo::Robustness;
const std::vector<media::VideoCodecProfile> kAllProfiles = {};
}  // namespace

namespace cdm {

#if defined(WIDEVINE_CDM_AVAILABLE)
bool IsWidevineAvailable(base::FilePath* cdm_path,
                         media::CdmCapability* capability) {
  static enum {
    NOT_CHECKED,
    FOUND,
    NOT_FOUND,
  } widevine_cdm_file_check = NOT_CHECKED;

  // TODO(neva): Get file path from configuration
  const std::string default_libwidevinecdm_path = "/usr/lib/libwidevinecdm.so";
  *cdm_path = base::FilePath::FromUTF8Unsafe(default_libwidevinecdm_path);

  if (widevine_cdm_file_check == NOT_CHECKED)
    widevine_cdm_file_check = base::PathExists(*cdm_path) ? FOUND : NOT_FOUND;

  if (widevine_cdm_file_check == FOUND) {
    // Add the supported codecs as if they came from the component manifest.
    // This list must match the CDM that is being bundled with Chrome.
    capability->video_codecs.emplace(media::VideoCodec::kVP8, kAllProfiles);
    capability->video_codecs.emplace(media::VideoCodec::kVP9, kAllProfiles);
#if BUILDFLAG(USE_PROPRIETARY_CODECS)
    capability->video_codecs.emplace(media::VideoCodec::kH264, kAllProfiles);
#endif  // BUILDFLAG(USE_PROPRIETARY_CODECS)

    // Add the supported encryption schemes as if they came from the
    // component manifest. This list must match the CDM that is being
    // bundled with Chrome.
    capability->encryption_schemes.insert(media::EncryptionScheme::kCenc);

    // Temporary session is always supported.
    capability->session_types.insert(media::CdmSessionType::kTemporary);
    // TODO(neva): Consider kHardwareSecureDecryption feature.
    return true;
  }
  return false;
}

#endif  // defined(WIDEVINE_CDM_AVAILABLE)

void AddContentDecryptionModules(std::vector<content::CdmInfo>& cdms) {
#if defined(WIDEVINE_CDM_AVAILABLE)
  base::FilePath cdm_path;
  media::CdmCapability capability;
  if (IsWidevineAvailable(&cdm_path, &capability)) {
    const base::Version version(WIDEVINE_CDM_VERSION_STRING);
    DCHECK(version.IsValid());

    cdms.push_back(content::CdmInfo(
        kWidevineKeySystem, Robustness::kSoftwareSecure, std::move(capability),
        /*supports_sub_key_systems=*/false, kWidevineCdmDisplayName,
        kWidevineCdmType, version, cdm_path));
  }
#endif  // defined(WIDEVINE_CDM_AVAILABLE)
}

}  // namespace cdm
