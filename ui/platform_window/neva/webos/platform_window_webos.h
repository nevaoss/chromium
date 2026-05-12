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

#ifndef UI_PLATFORM_WINDOW_NEVA_WEBOS_PLATFORM_WINDOW_WEBOS_H_
#define UI_PLATFORM_WINDOW_NEVA_WEBOS_PLATFORM_WINDOW_WEBOS_H_

#include "ui/gfx/geometry/size.h"

namespace ui {
namespace neva {

// webOS additions for platform window.
class PlatformWindowWebOS {
 public:
  void SetContentsSize(gfx::Size size) { contents_size_ = size; }
  gfx::Size GetContentsSize() const { return contents_size_; }
  bool HasValidContentsSize() const { return !contents_size_.IsEmpty(); }

 private:
  gfx::Size contents_size_;
};

}  // namespace neva
}  // namespace ui

#endif  // UI_PLATFORM_WINDOW_NEVA_WEBOS_PLATFORM_WINDOW_WEBOS_H_
