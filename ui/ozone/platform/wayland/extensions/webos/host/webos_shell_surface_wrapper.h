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

#ifndef UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WEBOS_SHELL_SURFACE_WRAPPER_H_
#define UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WEBOS_SHELL_SURFACE_WRAPPER_H_

#include <cstdint>
#include <vector>

#include "ui/ozone/platform/wayland/extensions/webos/common/wayland_webos_object.h"
#include "ui/ozone/platform/wayland/extensions/webos/host/wayland_shell_surface_wrapper.h"

namespace ui {

class WaylandConnection;
class WaylandWindowWebos;

enum class KeyMask: std::uint32_t;

// TODO(neva): (chr91) consider renaming to WebosShellToplevelWrapper
// to match upstream pattern
class WebosShellSurfaceWrapper : public WaylandShellSurfaceWrapper {
 public:
  WebosShellSurfaceWrapper(WaylandWindowWebos* wayland_window,
                           WaylandConnection* connection);
  WebosShellSurfaceWrapper(const WebosShellSurfaceWrapper&) = delete;
  WebosShellSurfaceWrapper& operator=(const WebosShellSurfaceWrapper&) = delete;
  ~WebosShellSurfaceWrapper() override;

  // ui::ShellToplevelWrapper
  bool Initialize() override;
  void SetMaximized() override;
  void UnSetMaximized() override;
  void SetFullscreen(WaylandOutput* wayland_output) override;
  void UnSetFullscreen() override;
  void SetMinimized() override;
  void SetDecoration(DecorationMode decoration) override;
  void Lock(WaylandOrientationLockType lock_type) override;
  void Unlock() override;
  void SetSystemModal(bool modal) override;

  void SetInputRegion(const std::vector<gfx::Rect>& region);
  void SetGroupKeyMask(KeyMask key_mask);
  void SetKeyMask(KeyMask key_mask, bool set);
  void SetWindowProperty(const std::string& name,
                         const std::string& value);

  // wl_webos_shell_surface listener
  // Called to notify a client that the surface state is changed.
  static void StateChanged(void* data,
                           wl_webos_shell_surface* webos_shell_surface,
                           uint32_t state);
  // Called to notify a client that the surface position is changed.
  static void PositionChanged(void* data,
                              wl_webos_shell_surface* webos_shell_surface,
                              int32_t x,
                              int32_t y);
  // Called by the compositor to request closing of the window.
  static void Close(void* data, wl_webos_shell_surface* webos_shell_surface);
  // Called to notify a client which surface areas are now exposed (visible).
  static void Exposed(void* data,
                      wl_webos_shell_surface* webos_shell_surface,
                      wl_array* rectangles);
  // Called to notify a client that the surface state is about to change.
  static void StateAboutToChange(void* data,
                                 wl_webos_shell_surface* webos_shell_surface,
                                 uint32_t state);

 private:
  WaylandWindowWebos* const wayland_window_;
  WaylandConnection* const connection_;
  std::uint32_t group_key_masks_;
  std::uint32_t applied_key_masks_;
  wl::Object<wl_webos_shell_surface> webos_shell_surface_;
};

}  // namespace ui

#endif  // UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WEBOS_SHELL_SURFACE_WRAPPER_H_
