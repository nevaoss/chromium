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

#ifndef UI_OZONE_PLATFORM_WAYLAND_HOST_WAYLAND_EXTENSIONS_H_
#define UI_OZONE_PLATFORM_WAYLAND_HOST_WAYLAND_EXTENSIONS_H_

#include <memory>

#include "ui/ozone/platform/wayland/common/wayland_object.h"

namespace ui {

class PlatformWindowDelegate;
class ShellToplevelWrapper;
class ShellPopupWrapper;
class WaylandConnection;
class WaylandWindow;

// Wayland extensions abstract interface to support extending of the Wayland
// protocol. Inherit it to provide your own Wayland extensions implementation.
class WaylandExtensions {
 public:
  WaylandExtensions() = default;
  WaylandExtensions(const WaylandExtensions&) = delete;
  WaylandExtensions& operator=(const WaylandExtensions&) = delete;
  virtual ~WaylandExtensions() = default;

  // Binds to the extensions interface(s). Can encapsulate binding of several
  // interfaces, defined by |interface|.
  virtual bool Bind(wl_registry* registry,
                    uint32_t name,
                    const char* interface,
                    uint32_t version) = 0;

  // Checks whether the extensions have bound shell object(s).
  virtual bool HasShellObject() const = 0;

  // Creates and returns shell toplevel wrapper object.
  virtual std::unique_ptr<ShellToplevelWrapper> CreateShellToplevel(
      WaylandWindow* window) = 0;

  // FIXME(neva): this API was intended for webOS which still doesn't provide
  // popup roles hence need to revise it for removement.
  // Creates and returns shell popup wrapper object.
  virtual std::unique_ptr<ShellPopupWrapper> CreateShellPopup(
      WaylandWindow* window) = 0;

  // Creates and returns extension-specific window object.
  virtual std::unique_ptr<WaylandWindow> CreateWaylandWindow(
      PlatformWindowDelegate* delegate,
      WaylandConnection* connection) = 0;
};

// Creates Wayland extensions.
std::unique_ptr<WaylandExtensions> CreateWaylandExtensions(
    WaylandConnection* connection);

}  // namespace ui

#endif  // UI_OZONE_PLATFORM_WAYLAND_HOST_WAYLAND_EXTENSIONS_H_
