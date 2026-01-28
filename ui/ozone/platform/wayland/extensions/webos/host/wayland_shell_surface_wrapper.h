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

#ifndef UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WAYLAND_SHELL_SURFACE_WRAPPER_H_
#define UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WAYLAND_SHELL_SURFACE_WRAPPER_H_

#include "ui/ozone/platform/wayland/extensions/webos/common/wayland_webos_object.h"
#include "ui/ozone/platform/wayland/host/xdg_toplevel.h"

namespace ui {

class WaylandConnection;
class WaylandOutput;
class WaylandWindowWebos;

// TODO(neva): (chr91) consider renaming to WaylandShellToplevelWrapper
// to match upstream pattern
class WaylandShellSurfaceWrapper : public XdgToplevel {
 public:
  WaylandShellSurfaceWrapper(WaylandWindowWebos* wayland_window,
                             WaylandConnection* connection);
  WaylandShellSurfaceWrapper(const WaylandShellSurfaceWrapper&) = delete;
  WaylandShellSurfaceWrapper& operator=(const WaylandShellSurfaceWrapper&) =
      delete;
  ~WaylandShellSurfaceWrapper() override;

  // ui::XdgToplevel:
  bool Initialize();
  void SetMaximized();
  void UnSetMaximized();
  void SetFullscreen(WaylandOutput* wayland_output);
  void UnSetFullscreen();
  void SetMinimized();
  void SurfaceMove(WaylandConnection* connection);
  void SurfaceResize(WaylandConnection* connection, uint32_t hittest);
  void SetTitle(const std::u16string& title);
  void AckConfigure(uint32_t serial);
  bool IsConfigured();
  void SetWindowGeometry(const gfx::Rect& bounds);
  void SetMinSize(int32_t width, int32_t height);
  void SetMaxSize(int32_t width, int32_t height);
  void SetAppId(const std::string& app_id);
  void ShowWindowMenu(WaylandConnection* connection, const gfx::Point& point);
  void SetIcon(const gfx::ImageSkia& icon);

  // wl_shell_surface listener
  static void Configure(void* data,
                        wl_shell_surface* shell_surface,
                        uint32_t edges,
                        int32_t width,
                        int32_t height);
  static void PopupDone(void* data, wl_shell_surface* shell_surface);
  static void Ping(void* data,
                   wl_shell_surface* shell_surface,
                   uint32_t serial);

 private:
  WaylandWindowWebos* const wayland_window_;
  WaylandConnection* const connection_;
  wl::Object<wl_shell_surface> shell_surface_;
};

}  // namespace ui

#endif  // UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WAYLAND_SHELL_SURFACE_WRAPPER_H_
