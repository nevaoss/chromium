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

#ifndef BASE_NEVA_NEVA_PATHS_H_
#define BASE_NEVA_NEVA_PATHS_H_

#include "base/files/file_path.h"
#include "build/build_config.h"

// This file declares path keys for the neva module. These can be used with
// the PathService to access various special directories and files.

namespace base {

enum {
  PATH_NEVA_START = 2000,
  TEMPORARY = PATH_NEVA_START,
#if BUILDFLAG(IS_WEBOS)
  FILE_MEDIA_CODEC_CAPABILITIES,
#endif  // BUILDFLAG(IS_WEBOS)
  PATH_NEVA_END
};

bool PathProviderNeva(int key, base::FilePath* result);

}  // namespace base

#endif  // BASE_NEVA_NEVA_PATHS_H_
