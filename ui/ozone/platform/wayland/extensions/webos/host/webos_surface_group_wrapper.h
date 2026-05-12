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

#ifndef UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WEBOS_SURFACE_GROUP_WRAPPER_H_
#define UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WEBOS_SURFACE_GROUP_WRAPPER_H_

#include "ui/ozone/platform/wayland/extensions/webos/common/wayland_webos_object.h"
#include "ui/ozone/platform/wayland/host/surface_group_wrapper.h"

namespace ui {

class WaylandConnection;
class WaylandWindow;

class WebosSurfaceGroupWrapper : public SurfaceGroupWrapper {
 public:
  WebosSurfaceGroupWrapper(wl_webos_surface_group* surface_group,
                           WaylandConnection* connection);
  WebosSurfaceGroupWrapper(const WebosSurfaceGroupWrapper&) = delete;
  WebosSurfaceGroupWrapper& operator=(const WebosSurfaceGroupWrapper&) = delete;
  ~WebosSurfaceGroupWrapper() override;

  // SurfaceGroupWrapper
  void CreateLayer(const std::string& layer_name,
                   std::int32_t z_index) override;
  void AttachToLayer(WaylandWindow* wayland_window,
                     const std::string& layer_name) override;
  void AttachToAnonymousLayer(WaylandWindow* wayland_window,
                              ZHint z_hint) override;
  void AllowAnonymousLayers(bool is_allowed) override;
  void Detach(WaylandWindow* wayland_window) override;
  void FocusOwner() override;
  void FocusLayer() override;

  // wl_webos_surface_group_listener
  // Called to notify a client that the group owner is destroyed.
  static void OwnerDestroyed(void* data,
                             wl_webos_surface_group* wl_webos_surface_group);

 private:
  wl::Object<wl_webos_surface_group> webos_surface_group_;
  std::string layer_name_;
  WaylandConnection* const connection_;
};

}  // namespace ui

#endif  // UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WEBOS_SURFACE_GROUP_WRAPPER_H_
