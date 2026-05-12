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

#include "ui/ozone/platform/wayland/extensions/webos/host/webos_shell_surface_wrapper.h"

#include <wayland-webos-shell-client-protocol.h>

#include "base/check.h"
#include "base/logging.h"
#include "base/notreached.h"
#include "neva/logging.h"
#include "ui/ozone/platform/wayland/extensions/webos/host/wayland_extensions_webos.h"
#include "ui/ozone/platform/wayland/extensions/webos/host/wayland_window_webos.h"
#include "ui/ozone/platform/wayland/host/wayland_connection.h"
#include "ui/views/widget/desktop_aura/neva/ui_constants.h"

namespace ui {

PlatformWindowState ToPlatformWindowState(uint32_t state) {
  switch (state) {
    case WL_WEBOS_SHELL_SURFACE_STATE_DEFAULT:
      return PlatformWindowState::kNormal;
    case WL_WEBOS_SHELL_SURFACE_STATE_MINIMIZED:
      return PlatformWindowState::kMinimized;
    case WL_WEBOS_SHELL_SURFACE_STATE_MAXIMIZED:
      return PlatformWindowState::kMaximized;
    case WL_WEBOS_SHELL_SURFACE_STATE_FULLSCREEN:
      return PlatformWindowState::kFullScreen;
    default:
      return PlatformWindowState::kUnknown;
  }
}

WebosShellSurfaceWrapper::WebosShellSurfaceWrapper(
    WaylandWindowWebos* wayland_window,
    WaylandConnection* connection)
    : WaylandShellSurfaceWrapper(wayland_window, connection),
      wayland_window_(wayland_window),
      connection_(connection),
      group_key_masks_(WL_WEBOS_SHELL_SURFACE_WEBOS_KEY_DEFAULT),
      applied_key_masks_(WL_WEBOS_SHELL_SURFACE_WEBOS_KEY_DEFAULT) {}

WebosShellSurfaceWrapper::~WebosShellSurfaceWrapper() = default;

bool WebosShellSurfaceWrapper::Initialize() {
  DCHECK(wayland_window_ && wayland_window_->GetWebosExtensions());
  WaylandExtensionsWebos* webos_extensions =
      wayland_window_->GetWebosExtensions();

  WaylandShellSurfaceWrapper::Initialize();

  webos_shell_surface_.reset(wl_webos_shell_get_shell_surface(
      webos_extensions->webos_shell(),
      wayland_window_->root_surface()->surface()));
  if (!webos_shell_surface_) {
    LOG(ERROR) << "Failed to create wl_webos_shell_surface";
    return false;
  }

  static const wl_webos_shell_surface_listener webos_shell_surface_listener = {
      WebosShellSurfaceWrapper::StateChanged,
      WebosShellSurfaceWrapper::PositionChanged,
      WebosShellSurfaceWrapper::Close, WebosShellSurfaceWrapper::Exposed,
      WebosShellSurfaceWrapper::StateAboutToChange};

  wl_webos_shell_surface_add_listener(webos_shell_surface_.get(),
                                      &webos_shell_surface_listener, this);

  wayland_window_->OnSurfaceConfigureEvent();

  return true;
}

void WebosShellSurfaceWrapper::SetMaximized() {
  wayland_window_->SetContentsBounds();
  wl_webos_shell_surface_set_state(webos_shell_surface_.get(),
                                   WL_WEBOS_SHELL_SURFACE_STATE_MAXIMIZED);
}

void WebosShellSurfaceWrapper::UnSetMaximized() {
  wl_webos_shell_surface_set_state(webos_shell_surface_.get(),
                                   WL_WEBOS_SHELL_SURFACE_STATE_DEFAULT);
}

void WebosShellSurfaceWrapper::SetFullscreen(WaylandOutput* wayland_output) {
  wayland_window_->SetContentsBounds();
  wl_webos_shell_surface_set_state(webos_shell_surface_.get(),
                                   WL_WEBOS_SHELL_SURFACE_STATE_FULLSCREEN);
}

void WebosShellSurfaceWrapper::UnSetFullscreen() {
  wl_webos_shell_surface_set_state(webos_shell_surface_.get(),
                                   WL_WEBOS_SHELL_SURFACE_STATE_DEFAULT);
}

void WebosShellSurfaceWrapper::SetMinimized() {
  wl_webos_shell_surface_set_state(webos_shell_surface_.get(),
                                   WL_WEBOS_SHELL_SURFACE_STATE_MINIMIZED);
  connection_->Flush();
}

void WebosShellSurfaceWrapper::SetDecoration(DecorationMode decoration) {
  NOTIMPLEMENTED_LOG_ONCE();
}

void WebosShellSurfaceWrapper::Lock(WaylandOrientationLockType lock_type) {
  NOTIMPLEMENTED_LOG_ONCE();
}

void WebosShellSurfaceWrapper::Unlock() {
  NOTIMPLEMENTED_LOG_ONCE();
}

void WebosShellSurfaceWrapper::SetInputRegion(
    const std::vector<gfx::Rect>& region) {
  wl::Object<wl_region> wlregion(
      wl_compositor_create_region(connection_->compositor()));

  for (const auto& reg : region)
    wl_region_add(wlregion.get(), reg.x(), reg.y(), reg.width(), reg.height());

  wl_surface_set_input_region(wayland_window_->root_surface()->surface(),
                              wlregion.get());
  wl_surface_commit(wayland_window_->root_surface()->surface());
}

void WebosShellSurfaceWrapper::SetGroupKeyMask(KeyMask key_mask) {
  std::uint32_t curr_key_masks = static_cast<std::uint32_t>(key_mask);

  if (group_key_masks_ == curr_key_masks)
    return;

  group_key_masks_ = curr_key_masks;
  wl_webos_shell_surface_set_key_mask(webos_shell_surface_.get(),
                                      group_key_masks_);
}

void WebosShellSurfaceWrapper::SetKeyMask(KeyMask key_mask, bool set) {
  std::uint32_t curr_key_mask = static_cast<std::uint32_t>(key_mask);
  std::uint32_t key_masks = set ? applied_key_masks_ | curr_key_mask
                                : applied_key_masks_ & ~curr_key_mask;
  if (key_masks == applied_key_masks_)
    return;

  applied_key_masks_ = key_masks;
  wl_webos_shell_surface_set_key_mask(webos_shell_surface_.get(), key_masks);
}

void WebosShellSurfaceWrapper::SetWindowProperty(const std::string& name,
                                                 const std::string& value) {
  wl_webos_shell_surface_set_property(webos_shell_surface_.get(), name.c_str(),
                                      value.c_str());
}
void WebosShellSurfaceWrapper::SetSystemModal(bool modal) {
  // TODO(neva): Implement SystemModal for webOS
  NOTIMPLEMENTED_LOG_ONCE();
}

// static
void WebosShellSurfaceWrapper::StateChanged(
    void* data,
    wl_webos_shell_surface* webos_shell_surface,
    uint32_t state) {
  WebosShellSurfaceWrapper* shell_surface_wrapper =
      static_cast<WebosShellSurfaceWrapper*>(data);
  DCHECK(shell_surface_wrapper);
  DCHECK(shell_surface_wrapper->wayland_window_);

  if (shell_surface_wrapper->wayland_window_)
    shell_surface_wrapper->wayland_window_->HandleStateChanged(
        ToPlatformWindowState(state));
}

void WebosShellSurfaceWrapper::PositionChanged(
    void* data,
    wl_webos_shell_surface* webos_shell_surface,
    int32_t x,
    int32_t y) {
  NOTIMPLEMENTED_LOG_ONCE();
}

void WebosShellSurfaceWrapper::Close(
    void* data,
    wl_webos_shell_surface* webos_shell_surface) {
  WebosShellSurfaceWrapper* shell_surface_wrapper =
      static_cast<WebosShellSurfaceWrapper*>(data);
  NEVA_DCHECK(shell_surface_wrapper);
  NEVA_DCHECK(shell_surface_wrapper->wayland_window_);
  shell_surface_wrapper->wayland_window_->HandleWindowHostClose();
}

void WebosShellSurfaceWrapper::Exposed(
    void* data,
    wl_webos_shell_surface* webos_shell_surface,
    wl_array* rectangles) {
  WebosShellSurfaceWrapper* shell_surface_wrapper =
      static_cast<WebosShellSurfaceWrapper*>(data);
  DCHECK(shell_surface_wrapper);
  DCHECK(shell_surface_wrapper->wayland_window_);
  if (shell_surface_wrapper->wayland_window_)
    shell_surface_wrapper->wayland_window_->HandleWindowHostExposed();
}

void WebosShellSurfaceWrapper::StateAboutToChange(
    void* data,
    wl_webos_shell_surface* webos_shell_surface,
    uint32_t state) {
  WebosShellSurfaceWrapper* shell_surface_wrapper =
      static_cast<WebosShellSurfaceWrapper*>(data);
  DCHECK(shell_surface_wrapper);
  DCHECK(shell_surface_wrapper->wayland_window_);

  if (shell_surface_wrapper->wayland_window_)
    shell_surface_wrapper->wayland_window_->HandleStateAboutToChange(
        ToPlatformWindowState(state));
}

}  // namespace ui
