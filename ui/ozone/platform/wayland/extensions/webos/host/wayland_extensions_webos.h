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

#ifndef UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WAYLAND_EXTENSIONS_WEBOS_H_
#define UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WAYLAND_EXTENSIONS_WEBOS_H_

struct wl_shell;
struct wl_webos_shell;

namespace ui {

class ExtendedInputWrapper;
class InputManagerWrapper;
class InputPanelManager;
class SurfaceGroupCompositorWrapper;
class WebosTextModelFactoryWrapper;

class WaylandExtensionsWebos {
 public:
  WaylandExtensionsWebos() = default;
  WaylandExtensionsWebos(const WaylandExtensionsWebos&) = delete;
  WaylandExtensionsWebos& operator=(const WaylandExtensionsWebos&) = delete;
  virtual ~WaylandExtensionsWebos() = default;

  // Returns extended input wrapper object.
  virtual ExtendedInputWrapper* GetExtendedInput() = 0;

  // Returns input manager wrapper object.
  virtual InputManagerWrapper* GetInputManager() = 0;

  // Returns input panel manager object.
  virtual InputPanelManager* GetInputPanelManager() = 0;

  // Returns surface group compositor wrapper object.
  virtual SurfaceGroupCompositorWrapper* GetSurfaceGroupCompositor() = 0;

  // Returns webOS text model factory wrapper object.
  virtual WebosTextModelFactoryWrapper* GetWebosTextModelFactory() = 0;

  // Getters for wayland shell objects specific for webOS platform.
  virtual wl_shell* shell() const = 0;
  virtual wl_webos_shell* webos_shell() const = 0;
};

}  // namespace ui

#endif  //UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WAYLAND_EXTENSIONS_WEBOS_H_