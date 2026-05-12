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

#ifndef UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_AGL_HOST_WAYLAND_EXTENSIONS_AGL_IMPL_H_
#define UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_AGL_HOST_WAYLAND_EXTENSIONS_AGL_IMPL_H_

#include "ui/ozone/platform/wayland/extensions/agl/host/wayland_extensions_agl.h"
#include "ui/ozone/platform/wayland/host/wayland_extensions.h"

namespace ui {

class AglShellWrapper;

// AGL extension implementation for webOS/Lite
class WaylandExtensionsAglImpl : public WaylandExtensions,
                                 public WaylandExtensionsAgl {
 public:
  explicit WaylandExtensionsAglImpl(WaylandConnection* connection);
  WaylandExtensionsAglImpl(const WaylandExtensionsAglImpl&) = delete;
  WaylandExtensionsAglImpl& operator=(const WaylandExtensionsAglImpl&) = delete;
  ~WaylandExtensionsAglImpl() override;

  // WaylandExtensions overrides
  bool Bind(wl_registry* registry,
            uint32_t name,
            const char* interface,
            uint32_t version) override;

  bool HasShellObject() const override;

  std::unique_ptr<ShellToplevelWrapper> CreateShellToplevel(
      WaylandWindow* window) override;

  std::unique_ptr<ShellPopupWrapper> CreateShellPopup(
      WaylandWindow* window) override;

  std::unique_ptr<WaylandWindow> CreateWaylandWindow(
      PlatformWindowDelegate* delegate,
      WaylandConnection* connection) override;

  // WaylandExtensionsAgl overrides
  AglShellWrapper* GetAglShell() override;

 private:
  std::unique_ptr<AglShellWrapper> agl_shell_;
  WaylandConnection* connection_;
};

}  // namespace ui

#endif  // UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_AGL_HOST_WAYLAND_EXTENSIONS_AGL_IMPL_H_