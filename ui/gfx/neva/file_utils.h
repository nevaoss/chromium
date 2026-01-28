// Copyright 2021 LG Electronics, Inc.
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

#ifndef UI_GFX_NEVA_FILE_UTILS_H_
#define UI_GFX_NEVA_FILE_UTILS_H_

#include "base/component_export.h"
#include "base/files/file_util.h"
#include "third_party/skia/include/core/SkBitmap.h"

namespace gfx {

COMPONENT_EXPORT(GFX)
SkBitmap* DecodeSkBitmapFromPNG(const base::FilePath& path);

}  // namespace gfx

#endif  // UI_GFX_NEVA_FILE_UTILS_H_
