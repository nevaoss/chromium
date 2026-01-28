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

#ifndef UI_OZONE_PLATFORM_WAYLAND_HOST_SURFACE_GROUP_COMPOSITOR_WRAPPER_H_
#define UI_OZONE_PLATFORM_WAYLAND_HOST_SURFACE_GROUP_COMPOSITOR_WRAPPER_H_

#include <memory>
#include <string>

namespace ui {

class SurfaceGroupWrapper;
class WaylandWindow;

// Wrapper for the surface group compositor interface used to manage groups
// of multi-client surfaces.
class SurfaceGroupCompositorWrapper {
 public:
  virtual ~SurfaceGroupCompositorWrapper() = default;

  // Creates and returns a specified surface group.
  virtual std::unique_ptr<SurfaceGroupWrapper> CreateSurfaceGroup(
      WaylandWindow* parent, const std::string& group_name) = 0;

  // Returns an existing surface group.
  virtual std::unique_ptr<SurfaceGroupWrapper> GetSurfaceGroup(
      const std::string& group_name) = 0;
};

}  // namespace ui

#endif  // UI_OZONE_PLATFORM_WAYLAND_HOST_SURFACE_GROUP_COMPOSITOR_WRAPPER_H_
