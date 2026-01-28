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

#ifndef UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_AGL_COMMON_WAYLAND_OBJECT_AGL_H_
#define UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_AGL_COMMON_WAYLAND_OBJECT_AGL_H_

#include "ui/ozone/platform/wayland/common/wayland_object.h"

struct agl_shell;

namespace wl {

template <>
struct ObjectTraits<agl_shell> {
  static const wl_interface* interface;
  static void (*deleter)(agl_shell*);
};

}  // namespace wl

#endif  // UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_AGL_COMMON_WAYLAND_OBJECT_AGL_H_