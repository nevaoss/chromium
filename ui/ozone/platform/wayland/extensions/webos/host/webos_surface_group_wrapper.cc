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

#include "ui/ozone/platform/wayland/extensions/webos/host/webos_surface_group_wrapper.h"

#include <wayland-webos-surface-group-client-protocol.h>

#include "ui/ozone/platform/wayland/extensions/webos/host/wayland_window_webos.h"
#include "ui/ozone/platform/wayland/host/wayland_connection.h"
#include "ui/ozone/platform/wayland/host/wayland_window.h"

namespace ui {

namespace {

constexpr std::uint32_t kAllowAnonymousLayers = 1;
constexpr std::uint32_t kDisallowAnonymousLayers = 0;

}  // namespace

WebosSurfaceGroupWrapper::WebosSurfaceGroupWrapper(
    wl_webos_surface_group* surface_group,
    WaylandConnection* connection)
    : connection_(connection) {
  webos_surface_group_.reset(surface_group);

  static const wl_webos_surface_group_listener webos_surface_group_listener = {
      WebosSurfaceGroupWrapper::OwnerDestroyed};

  wl_webos_surface_group_add_listener(webos_surface_group_.get(),
                                      &webos_surface_group_listener, this);
}

WebosSurfaceGroupWrapper::~WebosSurfaceGroupWrapper() = default;

void WebosSurfaceGroupWrapper::CreateLayer(const std::string& layer_name,
                                           std::int32_t z_index) {
  wl_webos_surface_group_create_layer(webos_surface_group_.get(),
                                      layer_name.c_str(), z_index);
}

void WebosSurfaceGroupWrapper::AttachToLayer(WaylandWindow* wayland_window,
                                             const std::string& layer_name) {
  layer_name_ = layer_name;
  wl_webos_surface_group_attach(webos_surface_group_.get(),
                                wayland_window->root_surface()->surface(),
                                layer_name.c_str());
}

void WebosSurfaceGroupWrapper::AttachToAnonymousLayer(
    WaylandWindow* wayland_window,
    ZHint z_hint) {
  wl_webos_surface_group_attach_anonymous(
      webos_surface_group_.get(), wayland_window->root_surface()->surface(),
      static_cast<std::uint32_t>(z_hint));
}

void WebosSurfaceGroupWrapper::AllowAnonymousLayers(bool is_allowed) {
  wl_webos_surface_group_allow_anonymous_layers(
      webos_surface_group_.get(),
      is_allowed ? kAllowAnonymousLayers : kDisallowAnonymousLayers);
}

void WebosSurfaceGroupWrapper::Detach(WaylandWindow* wayland_window) {
  wl_webos_surface_group_detach(webos_surface_group_.get(),
                                wayland_window->root_surface()->surface());
}

void WebosSurfaceGroupWrapper::FocusOwner() {
  wl_webos_surface_group_focus_owner(webos_surface_group_.get());
}

void WebosSurfaceGroupWrapper::FocusLayer() {
  wl_webos_surface_group_focus_layer(webos_surface_group_.get(),
                                     layer_name_.c_str());
}

// static
void WebosSurfaceGroupWrapper::OwnerDestroyed(
    void* data,
    wl_webos_surface_group* wl_webos_surface_group) {
  WebosSurfaceGroupWrapper* surface_group_wrapper =
      static_cast<WebosSurfaceGroupWrapper*>(data);
  DCHECK(surface_group_wrapper);
  DCHECK(surface_group_wrapper->connection_);

  WaylandWindowManager* window_manager =
      surface_group_wrapper->connection_->window_manager();

  if (window_manager) {
    // Need to get all windows from the window manager because it is not clear
    // what window owns this surface.
    auto windows_all = window_manager->GetAllWindows();
    for (auto& window : windows_all) {
      // Detach and destroy Surface and close window after destroy owner surface
      if (window)
        window->HandleWindowClose(surface_group_wrapper);
    }
  }
}

}  // namespace ui
