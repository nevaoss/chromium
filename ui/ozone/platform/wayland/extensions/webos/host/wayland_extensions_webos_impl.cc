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

#include "ui/ozone/platform/wayland/extensions/webos/host/wayland_extensions_webos_impl.h"

#include <algorithm>
#include <cstring>
#include <wayland-webos-extension-client-protocol.h>

#include "base/logging.h"
#include "ui/ozone/platform/wayland/extensions/webos/host/webos_shell_surface_wrapper.h"
#include "ui/ozone/platform/wayland/host/shell_popup_wrapper.h"

namespace ui {

namespace {

constexpr uint32_t kMaxWlShellVersion = 1;
constexpr uint32_t kMaxWebosInputManagerVersion = 1;
constexpr uint32_t kMaxWebosShellVersion = 1;
constexpr uint32_t kMaxWebosSurfaceGroupCompositorVersion = 1;
constexpr uint32_t kMaxWebosTextModelFactoryVersion = 1;
constexpr uint32_t kMaxWebosXInputExtensionVersion = 1;

}  // namespace

WaylandExtensionsWebosImpl::WaylandExtensionsWebosImpl(
    WaylandConnection* connection)
    : connection_(connection) {}

WaylandExtensionsWebosImpl::~WaylandExtensionsWebosImpl() = default;

bool WaylandExtensionsWebosImpl::Bind(wl_registry* registry,
                                      uint32_t name,
                                      const char* interface,
                                      uint32_t version) {
  if (!wl_shell_ && strcmp(interface, "wl_shell") == 0) {
    wl_shell_ = wl::Bind<wl_shell>(registry, name,
                                   std::min(version, kMaxWlShellVersion));

    if (!wl_shell_) {
      LOG(ERROR) << "Failed to bind to wl_shell global";
      return false;
    }
    return true;
  } else if (!webos_shell_ && strcmp(interface, "wl_webos_shell") == 0) {
    webos_shell_ = wl::Bind<wl_webos_shell>(
        registry, name, std::min(version, kMaxWebosShellVersion));

    if (!webos_shell_) {
      LOG(ERROR) << "Failed to bind to wl_webos_shell global";
      return false;
    }
    return true;
  } else if (!surface_group_compositor_ &&
             strcmp(interface, "wl_webos_surface_group_compositor") == 0) {
    wl::Object<wl_webos_surface_group_compositor> surface_group_compositor =
        wl::Bind<wl_webos_surface_group_compositor>(
            registry, name,
            std::min(version, kMaxWebosSurfaceGroupCompositorVersion));

    if (!surface_group_compositor) {
      LOG(ERROR)
          << "Failed to bind to wl_webos_surface_group_compositor global";
      return false;
    }

    surface_group_compositor_ =
        std::make_unique<WebosSurfaceGroupCompositorWrapper>(
            surface_group_compositor.release(), connection_);

    return true;
  } else if (!text_model_factory_ &&
             strcmp(interface, "text_model_factory") == 0) {
    wl::Object<struct text_model_factory> factory =
        wl::Bind<struct text_model_factory>(
            registry, name,
            std::min(version, kMaxWebosTextModelFactoryVersion));

    if (!factory) {
      LOG(ERROR) << "Failed to bind to text_model_factory global";
      return false;
    }

    text_model_factory_ =
        std::make_unique<WebosTextModelFactoryWrapper>(factory.release());

    if (!input_panel_manager_)
      input_panel_manager_ =
          std::make_unique<WebosInputPanelManager>(connection_);
  } else if (!extended_input_ &&
             strcmp(interface, "wl_webos_xinput_extension") == 0) {
    wl::Object<wl_webos_xinput_extension> xinput_extension =
        wl::Bind<wl_webos_xinput_extension>(
            registry, name, std::min(version, kMaxWebosXInputExtensionVersion));

    if (!xinput_extension) {
      LOG(ERROR) << "Failed to bind to wl_webos_xinput_extension global";
      return false;
    }

    wl_webos_xinput* xinput =
        wl_webos_xinput_extension_register_input(xinput_extension.release());

    if (xinput)
      extended_input_ = std::make_unique<WebosExtendedInputWrapper>(xinput);
  } else if (!input_manager_ &&
             strcmp(interface, "wl_webos_input_manager") == 0) {
    wl::Object<wl_webos_input_manager> input_manager =
        wl::Bind<wl_webos_input_manager>(
            registry, name, std::min(version, kMaxWebosInputManagerVersion));

    if (!input_manager) {
      LOG(ERROR) << "Failed to bind to wl_webos_input_manager global";
      return false;
    }

    input_manager_ = std::make_unique<WebosInputManagerWrapper>(
        input_manager.release(), connection_);
  }

  return false;
}

bool WaylandExtensionsWebosImpl::HasShellObject() const {
  return !!wl_shell_ && !!webos_shell_;
}

std::unique_ptr<ShellToplevelWrapper>
WaylandExtensionsWebosImpl::CreateShellToplevel(WaylandWindow* window) {
  if (!!wl_shell_ && !!webos_shell_) {
    auto webos_window = static_cast<WaylandWindowWebos*>(window);
    return std::make_unique<WebosShellSurfaceWrapper>(webos_window,
                                                      connection_);
  }

  return nullptr;
}

std::unique_ptr<ShellPopupWrapper> WaylandExtensionsWebosImpl::CreateShellPopup(
    WaylandWindow* window) {
  return nullptr;
}

std::unique_ptr<WaylandWindow> WaylandExtensionsWebosImpl::CreateWaylandWindow(
    PlatformWindowDelegate* delegate,
    WaylandConnection* connection) {
  return std::make_unique<WaylandWindowWebos>(delegate, connection, this);
}

ExtendedInputWrapper* WaylandExtensionsWebosImpl::GetExtendedInput() {
  return extended_input_.get();
}

InputManagerWrapper* WaylandExtensionsWebosImpl::GetInputManager() {
  return input_manager_.get();
}

InputPanelManager* WaylandExtensionsWebosImpl::GetInputPanelManager() {
  return input_panel_manager_.get();
}

SurfaceGroupCompositorWrapper*
WaylandExtensionsWebosImpl::GetSurfaceGroupCompositor() {
  return surface_group_compositor_.get();
}

WebosTextModelFactoryWrapper*
WaylandExtensionsWebosImpl::GetWebosTextModelFactory() {
  return text_model_factory_.get();
}

std::unique_ptr<WaylandExtensions> CreateWaylandExtensions(
    WaylandConnection* connection) {
  return std::make_unique<WaylandExtensionsWebosImpl>(connection);
}

}  // namespace ui
