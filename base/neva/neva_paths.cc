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

#include "base/neva/neva_paths.h"

#include "base/files/file_util.h"
#include "build/build_config.h"

namespace base {

namespace {
#if BUILDFLAG(IS_WEBOS)
const base::FilePath::CharType kFilePathMediaCodecCapabilities[] =
    FILE_PATH_LITERAL("/etc/umediaserver/device_codec_capability_config.json");
#endif  // BUILDFLAG(IS_WEBOS)
}  // namespace

bool PathProviderNeva(int key, base::FilePath* result) {
  switch (key) {
#if BUILDFLAG(IS_WEBOS)
    case FILE_MEDIA_CODEC_CAPABILITIES:
      *result = base::FilePath(kFilePathMediaCodecCapabilities);
      return true;
#endif  // BUILDFLAG(IS_WEBOS)
    default:
      return false;
  }
}

}  // namespace base
