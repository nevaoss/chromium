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

#include "ui/ozone/platform/wayland/extensions/webos/host/webos_surface_group_compositor_wrapper.h"

#include <wayland-webos-surface-group-client-protocol.h>

#include "ui/ozone/platform/wayland/extensions/webos/host/webos_surface_group_wrapper.h"
#include "ui/ozone/platform/wayland/host/wayland_window.h"

namespace ui {

WebosSurfaceGroupCompositorWrapper::WebosSurfaceGroupCompositorWrapper(
    wl_webos_surface_group_compositor* surface_group_compositor,
    WaylandConnection* connection)
    : webos_surface_group_compositor_(surface_group_compositor),
      connection_(connection) {}

WebosSurfaceGroupCompositorWrapper::~WebosSurfaceGroupCompositorWrapper() =
    default;

std::unique_ptr<SurfaceGroupWrapper>
WebosSurfaceGroupCompositorWrapper::CreateSurfaceGroup(
    WaylandWindow* parent,
    const std::string& group_name) {
  wl_webos_surface_group* webos_surface_group =
      wl_webos_surface_group_compositor_create_surface_group(
          webos_surface_group_compositor_.get(),
          parent->root_surface()->surface(), group_name.c_str());

  return std::make_unique<WebosSurfaceGroupWrapper>(webos_surface_group,
                                                    connection_);
}

std::unique_ptr<SurfaceGroupWrapper>
WebosSurfaceGroupCompositorWrapper::GetSurfaceGroup(
    const std::string& group_name) {
  wl_webos_surface_group* webos_surface_group =
      wl_webos_surface_group_compositor_get_surface_group(
          webos_surface_group_compositor_.get(), group_name.c_str());

  return std::make_unique<WebosSurfaceGroupWrapper>(webos_surface_group,
                                                    connection_);
}

}  // namespace ui
