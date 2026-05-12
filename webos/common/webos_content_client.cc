// Copyright 2016-2019 LG Electronics, Inc.
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

#include "webos/common/webos_content_client.h"

#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/common/content_plugin_info.h"
#include "ppapi/shared_impl/ppapi_permissions.h"

#if defined(ENABLE_PLAYREADY_CDM)
#include "third_party/playready/cdm/playready_cdm_version.h"
#endif

namespace webos {

void WebOSContentClient::AddPlugins(
    std::vector<content::ContentPluginInfo>* plugins) {
  base::FilePath path;
#if defined(ENABLE_PLAYREADY_CDM) && defined(ENABLE_PEPPER_CDMS)
  static bool skip_playready_cdm_file_check = false;
  const char* cdm_lib_path = getenv("CDM_LIB_PATH");
  if (!cdm_lib_path) {
    return;
  }
  std::string playready_lib_path(cdm_lib_path);
  std::string playready_lib_file("/libplayreadycdmadapter.so");
  path = base::FilePath(playready_lib_path + playready_lib_file);
  if (skip_playready_cdm_file_check || base::PathExists(path)) {
    content::ContentPluginInfo playready_cdm;
    playready_cdm.is_out_of_process = true;
    playready_cdm.path = path;
    playready_cdm.name = kPlayReadyCdmDisplayName;
    playready_cdm.description =
        kPlayReadyCdmDescription +
        std::string(" (version: ") +
        PLAYREADY_CDM_VERSION_STRING +
        ")";
    playready_cdm.version = PLAYREADY_CDM_VERSION_STRING;
    content::WebPluginMimeType playready_cdm_mime_type(
        kPlayReadyCdmPluginMimeType,
        "",
        kPlayReadyCdmDisplayName);

    std::vector<std::string> codecs;
    codecs.push_back(kCdmSupportedCodecVorbis);
    codecs.push_back(kCdmSupportedCodecVp8);
    codecs.push_back(kCdmSupportedCodecVp9);
#if defined(USE_PROPRIETARY_CODECS)
    codecs.push_back(kCdmSupportedCodecAac);
    codecs.push_back(kCdmSupportedCodecAvc1);
#endif

    const char playready_codecs_delimiter[] = { kCdmSupportedCodecsValueDelimiter };
    std::string codec_string =
        base::JoinString(codecs, playready_codecs_delimiter);
    playready_cdm_mime_type.additional_param_names.push_back(
        base::ASCIIToUTF16(kCdmSupportedCodecsParamName));
    playready_cdm_mime_type.additional_param_values.push_back(
        base::ASCIIToUTF16(codec_string));

    playready_cdm.mime_types.push_back(playready_cdm_mime_type);
    playready_cdm.permissions = ppapi::PERMISSION_DEV |
                                ppapi::PERMISSION_PRIVATE;

    plugins->push_back(playready_cdm);

    skip_playready_cdm_file_check = true;
  }
#endif
}

}  // namespace webos
