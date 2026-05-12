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

#ifndef UI_OZONE_PLATFORM_WAYLAND_HOST_SURFACE_GROUP_WRAPPER_H_
#define UI_OZONE_PLATFORM_WAYLAND_HOST_SURFACE_GROUP_WRAPPER_H_

#include <cstdint>
#include <string>

namespace ui {

class WaylandWindow;

// Z-index hint for an anonymous surface group layer.
enum class ZHint: std::uint32_t {
  kBelow = 0,  // below the root (owner) surface and the lowermost named layer
  kAbove = 1,  // above the root (owner) surface and the uppermost named layer
  kTop = 2  // above all of the (existing) anonymous layers
};

// Wrapper for the surface group interface used to manage a dedicated surface
// group instance.
class SurfaceGroupWrapper {
 public:
  virtual ~SurfaceGroupWrapper() = default;

  // Creates a specified layer within the surface group.
  virtual void CreateLayer(const std::string& layer_name,
                           std::int32_t z_index) = 0;

  // Attaches a surface to a specified layer within the surface group.
  virtual void AttachToLayer(WaylandWindow* wayland_window,
                             const std::string& layer_name) = 0;

  // Attaches a surface to an anonymous layer within the surface group.
  virtual void AttachToAnonymousLayer(WaylandWindow* wayland_window,
                                      ZHint z_hint) = 0;

  // Specifies whether anonymous layers are allowed within the surface group.
  virtual void AllowAnonymousLayers(bool is_allowed) = 0;

  // Detaches a surface from the surface group.
  virtual void Detach(WaylandWindow* wayland_window) = 0;

  // Switches input focus to the owner surface (created the surface group).
  virtual void FocusOwner() = 0;

  // Switches input focus to a layer (with an attached surface).
  virtual void FocusLayer() = 0;
};

}  // namespace ui

#endif  // UI_OZONE_PLATFORM_WAYLAND_HOST_SURFACE_GROUP_WRAPPER_H_
