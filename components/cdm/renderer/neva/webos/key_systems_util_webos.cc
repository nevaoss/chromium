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

#include "components/cdm/renderer/neva/key_systems_util.h"

#include "components/cdm/renderer/widevine_key_system_info.h"
#include "content/public/renderer/key_system_support.h"
#include "media/base/eme_constants.h"
#include "media/base/video_codecs.h"

#include "widevine_cdm_version.h"  // In SHARED_INTERMEDIATE_DIR.

namespace cdm {

#if defined(WIDEVINE_CDM_AVAILABLE)
void AddWidevine(const media::mojom::KeySystemCapabilityPtr& capability,
                 media::KeySystemInfos* key_systems) {
  const auto& encryption_schemes =
      capability->sw_secure_capability->encryption_schemes;

  using media::SupportedCodecs;
  SupportedCodecs supported_codecs = media::EME_CODEC_NONE;

  // Audio webm codecs are always supported.
  supported_codecs |= media::EME_CODEC_OPUS;
  supported_codecs |= media::EME_CODEC_VORBIS;

#if BUILDFLAG(USE_PROPRIETARY_CODECS)
  supported_codecs |= media::EME_CODEC_AAC;
#endif  // BUILDFLAG(USE_PROPRIETARY_CODECS)

  // Video codecs are determined by what was registered for the CDM.
  for (const auto& codec : capability->sw_secure_capability->video_codecs) {
    switch (codec.first) {
      case media::VideoCodec::kVP8:
        supported_codecs |= media::EME_CODEC_VP8;
        break;
      case media::VideoCodec::kVP9:
        supported_codecs |= media::EME_CODEC_VP9_PROFILE0;
        supported_codecs |= media::EME_CODEC_VP9_PROFILE2;
        break;
#if BUILDFLAG(USE_PROPRIETARY_CODECS)
      case media::VideoCodec::kH264:
        supported_codecs |= media::EME_CODEC_AVC1;
        break;
#endif  // BUILDFLAG(USE_PROPRIETARY_CODECS)
      default:
        LOG(INFO) << "Unexpected supported codec: "
                  << GetCodecName(codec.first);
        break;
    }
  }

  base::flat_set<media::CdmSessionType> session_types;
  session_types.insert(media::CdmSessionType::kTemporary);

  using Robustness = cdm::WidevineKeySystemInfo::Robustness;

  using media::EmeFeatureSupport;

  key_systems->emplace_back(new cdm::WidevineKeySystemInfo(
      supported_codecs, encryption_schemes, session_types,
      supported_codecs,                   // Hardware secure codecs.
      std::move(encryption_schemes),      // Hardware secure encryption schemes.
      std::move(session_types),           // Hardware secure session types.
      Robustness::HW_SECURE_CRYPTO,       // Maximum audio robustness.
      Robustness::HW_SECURE_ALL,          // Maximum video robustness.
      EmeFeatureSupport::ALWAYS_ENABLED,  // Persistent state.
      EmeFeatureSupport::REQUESTABLE));   // Distinctive identifier.
}
#endif  // defined(WIDEVINE_CDM_AVAILABLE)

void OnKeySystemSupportUpdated(
    media::GetSupportedKeySystemsCB cb,
    content::KeySystemCapabilityPtrMap key_system_capabilities) {
  media::KeySystemInfos key_systems;
  for (const auto& entry : key_system_capabilities) {
    const auto& key_system = entry.first;
    const auto& capability = entry.second;

#if defined(WIDEVINE_CDM_AVAILABLE)
    if (key_system == kWidevineKeySystem) {
      AddWidevine(capability, &key_systems);
      continue;
    }
#endif  // defined(WIDEVINE_CDM_AVAILABLE)

    LOG(INFO) << "Ignore key system: " << key_system;
  }
  cb.Run(std::move(key_systems));
}

void AddSupportedKeySystems(media::GetSupportedKeySystemsCB cb) {
  content::ObserveKeySystemSupportUpdate(
      base::BindRepeating(&OnKeySystemSupportUpdated, std::move(cb)));
}

}  // namespace cdm
