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

#ifndef UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WAYLAND_WINDOW_WEBOS_H_
#define UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WAYLAND_WINDOW_WEBOS_H_

#include "ui/ozone/platform/wayland/host/wayland_toplevel_window.h"

namespace ui {

class SurfaceGroupWrapper;
class WaylandConnection;
class WaylandExtensionsWebos;
class WaylandInputMethodContext;

class WaylandWindowWebos : public WaylandToplevelWindow {
 public:
  explicit WaylandWindowWebos(PlatformWindowDelegate* delegate,
                              WaylandConnection* connection,
                              WaylandExtensionsWebos* webos_extensions);
  WaylandWindowWebos(const WaylandWindowWebos&) = delete;
  WaylandWindowWebos& operator=(const WaylandWindowWebos&) = delete;
  ~WaylandWindowWebos() override;

  WaylandInputMethodContext* GetInputMethodContext();
  WaylandExtensionsWebos* GetWebosExtensions();

  void HandleStateChanged(PlatformWindowState state);
  void HandleWindowHostClose();
  void HandleWindowHostExposed();
  void HandleStateAboutToChange(PlatformWindowState state);
  void HandleCursorVisibilityChanged(bool is_visible);
  void HandleInputPanelRectangleChange(std::int32_t x,
                                       std::int32_t y,
                                       std::uint32_t width,
                                       std::uint32_t height);

  // ui::PlatformWindow override
  void SetInputRegion(std::optional<std::vector<gfx::Rect>> region_px) override;

  // TODO: introduce neva::PlatformWindowWebos for webOS specifics
  // neva::PlatformWindow overrides
  void CreateGroup(const ui::WindowGroupConfiguration& config) override;
  void AttachToGroup(const std::string& name,
                     const std::string& layer) override;
  void FocusGroupOwner() override;
  void FocusGroupLayer() override;
  void DetachGroup() override;
  void ShowInputPanel() override;
  void HideInputPanel() override;
  void SetTextInputInfo(const ui::TextInputInfo& text_input_info) override;
  void SetSurroundingText(const std::string& text,
                          std::size_t cursor_position,
                          std::size_t anchor_position) override;
  void XInputActivate(const std::string& type) override;
  void XInputDeactivate() override;
  void XInputInvokeAction(std::uint32_t keysym,
                          XInputKeySymbolType symbol_type,
                          XInputEventType event_type) override;
  void SetCursorVisibility(bool visible) override;
  void SetGroupKeyMask(KeyMask key_mask) override;
  void SetKeyMask(KeyMask key_mask, bool set) override;
  void SetWindowProperty(const std::string& name,
                         const std::string& value) override;
  void SetContentsBounds();
  void HandleWindowClose(SurfaceGroupWrapper* surface_group) override;

 private:
  WaylandExtensionsWebos* webos_extensions_;

  // Wrapper around surface group object.
  std::unique_ptr<SurfaceGroupWrapper> surface_group_;
};

}  // namespace ui

#endif  // UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WAYLAND_WINDOW_WEBOS_H_
