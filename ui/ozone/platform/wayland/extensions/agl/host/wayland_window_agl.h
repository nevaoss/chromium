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
//

#ifndef UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_AGL_HOST_WAYLAND_WINDOW_AGL_H_
#define UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_AGL_HOST_WAYLAND_WINDOW_AGL_H_

#include "ui/ozone/platform/wayland/host/wayland_toplevel_window.h"

namespace ui {

class WaylandExtensionsAgl;

class WaylandWindowAgl : public WaylandToplevelWindow {
 public:
  explicit WaylandWindowAgl(PlatformWindowDelegate* delegate,
                            WaylandConnection* connection,
                            WaylandExtensionsAgl* agl_extensions);
  WaylandWindowAgl(const WaylandWindowAgl&) = delete;
  WaylandWindowAgl& operator=(const WaylandWindowAgl&) = delete;
  ~WaylandWindowAgl() override;

  // Overrides PlatformWindowAgl
  void SetAglActivateApp(const std::string& app) override;
  void SetAglAppId(const std::string& title) override;
  void SetAglReady() override;
  void SetAglBackground() override;
  void SetAglPanel(uint32_t edge) override;

 private:
  WaylandExtensionsAgl* agl_extensions_;
};

}  // namespace ui

#endif  // UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_AGL_HOST_WAYLAND_WINDOW_AGL_H_
