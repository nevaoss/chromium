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

#include "ui/ozone/platform/wayland/extensions/webos/host/webos_surface_group_layer_wrapper.h"

#include <wayland-webos-surface-group-client-protocol.h>

#include "base/notreached.h"

namespace ui {

WebosSurfaceGroupLayerWrapper::WebosSurfaceGroupLayerWrapper(
    wl_webos_surface_group_layer* surface_group_layer)
    : webos_surface_group_layer_(surface_group_layer) {
  static const wl_webos_surface_group_layer_listener
      webos_surface_group_layer_listener = {
          WebosSurfaceGroupLayerWrapper::SurfaceAttached,
          WebosSurfaceGroupLayerWrapper::SurfaceDetached};

  wl_webos_surface_group_layer_add_listener(webos_surface_group_layer_.get(),
                                            &webos_surface_group_layer_listener,
                                            this);
}

void WebosSurfaceGroupLayerWrapper::SetZIndex(std::int32_t z_index) {
  wl_webos_surface_group_layer_set_z_index(webos_surface_group_layer_.get(),
                                           z_index);
}

void WebosSurfaceGroupLayerWrapper::Destroy() {
  wl_webos_surface_group_layer_destroy(webos_surface_group_layer_.get());
}

// static
void WebosSurfaceGroupLayerWrapper::SurfaceAttached(
    void* data,
    wl_webos_surface_group_layer* wl_webos_surface_group_layer) {
  NOTIMPLEMENTED_LOG_ONCE();
}

void WebosSurfaceGroupLayerWrapper::SurfaceDetached(
    void* data,
    wl_webos_surface_group_layer* wl_webos_surface_group_layer) {
  NOTIMPLEMENTED_LOG_ONCE();
}

}  // namespace ui
