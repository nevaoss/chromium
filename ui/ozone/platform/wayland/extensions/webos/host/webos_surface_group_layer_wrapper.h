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
//

#ifndef UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WEBOS_SURFACE_GROUP_LAYER_WRAPPER_H_
#define UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WEBOS_SURFACE_GROUP_LAYER_WRAPPER_H_

#include <cstdint>

#include "ui/ozone/platform/wayland/extensions/webos/common/wayland_webos_object.h"

namespace ui {

// Wrapper for the surface group layer interface used to manage a dedicated
// surface group layer instance.
class WebosSurfaceGroupLayerWrapper {
 public:
  explicit WebosSurfaceGroupLayerWrapper(
      wl_webos_surface_group_layer* surface_group_layer);
  WebosSurfaceGroupLayerWrapper(const WebosSurfaceGroupLayerWrapper&) = delete;
  WebosSurfaceGroupLayerWrapper& operator=(
      const WebosSurfaceGroupLayerWrapper&) = delete;
  ~WebosSurfaceGroupLayerWrapper() = default;

  // Updates the Z-index for the group layer.
  void SetZIndex(std::int32_t z_index);
  // Destroys the group layer.
  void Destroy();

  // wl_webos_surface_group_layer_listener
  // Called to notify a client that a surface is attached to the group layer.
  static void SurfaceAttached(
      void* data,
      wl_webos_surface_group_layer* wl_webos_surface_group_layer);
  // Called to notify a client that a surface is detached from the group layer.
  static void SurfaceDetached(
      void* data,
      wl_webos_surface_group_layer* wl_webos_surface_group_layer);

 private:
  wl::Object<wl_webos_surface_group_layer> webos_surface_group_layer_;
};

}  // namespace ui

#endif  // UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WEBOS_SURFACE_GROUP_LAYER_WRAPPER_H_
