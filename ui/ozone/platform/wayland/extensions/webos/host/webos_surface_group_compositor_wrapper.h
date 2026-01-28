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

#ifndef UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WEBOS_SURFACE_GROUP_COMPOSITOR_WRAPPER_H_
#define UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WEBOS_SURFACE_GROUP_COMPOSITOR_WRAPPER_H_

#include "ui/ozone/platform/wayland/extensions/webos/common/wayland_webos_object.h"
#include "ui/ozone/platform/wayland/host/surface_group_compositor_wrapper.h"

namespace ui {

class SurfaceGroupWrapper;
class WaylandConnection;
class WaylandWindow;

class WebosSurfaceGroupCompositorWrapper
    : public SurfaceGroupCompositorWrapper {
 public:
  WebosSurfaceGroupCompositorWrapper(
      wl_webos_surface_group_compositor* surface_group_compositor,
      WaylandConnection* connection);
  WebosSurfaceGroupCompositorWrapper(
      const WebosSurfaceGroupCompositorWrapper&) = delete;
  WebosSurfaceGroupCompositorWrapper& operator=(
      const WebosSurfaceGroupCompositorWrapper&) = delete;
  ~WebosSurfaceGroupCompositorWrapper() override;

  // SurfaceGroupCompositorWrapper
  std::unique_ptr<SurfaceGroupWrapper> CreateSurfaceGroup(
      WaylandWindow* parent,
      const std::string& group_name) override;
  std::unique_ptr<SurfaceGroupWrapper> GetSurfaceGroup(
      const std::string& group_name) override;

 private:
  wl::Object<wl_webos_surface_group_compositor> webos_surface_group_compositor_;

  WaylandConnection* const connection_;
};

}  // namespace ui

#endif  // UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WEBOS_SURFACE_GROUP_COMPOSITOR_WRAPPER_H_
