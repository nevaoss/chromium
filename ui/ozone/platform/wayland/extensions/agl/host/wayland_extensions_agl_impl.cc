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

#include "ui/ozone/platform/wayland/extensions/agl/host/wayland_extensions_agl_impl.h"

#include <cstring>

#include "base/command_line.h"
#include "base/logging.h"
#include "ui/base/ui_base_neva_switches.h"
#include "ui/ozone/platform/wayland/extensions/agl/common/wayland_object_agl.h"
#include "ui/ozone/platform/wayland/extensions/agl/host/agl_shell_wrapper.h"
#include "ui/ozone/platform/wayland/extensions/agl/host/wayland_window_agl.h"
#include "ui/ozone/platform/wayland/host/extended_input_wrapper.h"
#include "ui/ozone/platform/wayland/host/input_manager_wrapper.h"
#include "ui/ozone/platform/wayland/host/input_panel_manager.h"
#include "ui/ozone/platform/wayland/host/shell_popup_wrapper.h"
#include "ui/ozone/platform/wayland/host/shell_surface_wrapper.h"
#include "ui/ozone/platform/wayland/host/surface_group_compositor_wrapper.h"
#include "ui/ozone/platform/wayland/host/wayland_connection.h"

namespace ui {

namespace {

constexpr uint32_t kMaxAglShellExtensionVersion = 1;

}  // namespace

WaylandExtensionsAglImpl::WaylandExtensionsAglImpl(
    WaylandConnection* connection)
    : connection_(connection) {}

WaylandExtensionsAglImpl::~WaylandExtensionsAglImpl() = default;

bool WaylandExtensionsAglImpl::Bind(wl_registry* registry,
                                    uint32_t name,
                                    const char* interface,
                                    uint32_t version) {
  bool should_use_agl_shell =
      base::CommandLine::ForCurrentProcess()->HasSwitch(switches::kUseAglShell);

  if (should_use_agl_shell && !agl_shell_ && (strcmp(interface, "agl_shell") == 0)) {
    wl::Object<agl_shell> aglshell =
        wl::Bind<agl_shell>(registry, name, kMaxAglShellExtensionVersion);
    if (!aglshell) {
      LOG(ERROR) << "Failed to bind to agl_shell global";
      return false;
    }
    agl_shell_ =
        std::make_unique<AglShellWrapper>(aglshell.release(), connection_);
    return true;
  }

  return false;
}

bool WaylandExtensionsAglImpl::HasShellObject() const {
  return !!agl_shell_;
}

std::unique_ptr<ShellToplevelWrapper>
WaylandExtensionsAglImpl::CreateShellToplevel(WaylandWindow* window) {
  return nullptr;
}

std::unique_ptr<ShellPopupWrapper> WaylandExtensionsAglImpl::CreateShellPopup(
    WaylandWindow* window) {
  return nullptr;
}

std::unique_ptr<WaylandWindow> WaylandExtensionsAglImpl::CreateWaylandWindow(
    PlatformWindowDelegate* delegate,
    WaylandConnection* connection) {
  return std::make_unique<WaylandWindowAgl>(delegate, connection, this);
}

AglShellWrapper* WaylandExtensionsAglImpl::GetAglShell() {
  return agl_shell_.get();
}

std::unique_ptr<WaylandExtensions> CreateWaylandExtensions(
    WaylandConnection* connection) {
  return std::make_unique<WaylandExtensionsAglImpl>(connection);
}

}  // namespace ui