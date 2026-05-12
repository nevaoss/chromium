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

#include "ui/ozone/platform/wayland/extensions/agl/host/agl_shell_wrapper.h"

#include <agl-shell-client-protocol.h>

#include "ui/ozone/platform/wayland/host/wayland_connection.h"
#include "ui/ozone/platform/wayland/host/wayland_output_manager.h"
#include "ui/ozone/platform/wayland/host/wayland_window.h"

namespace ui {

AglShellWrapper::AglShellWrapper(agl_shell* agl_shell,
                                 WaylandConnection* wayland_connection)
    : agl_shell_(agl_shell), connection_(wayland_connection) {}

AglShellWrapper::~AglShellWrapper() = default;

void AglShellWrapper::SetAglActivateApp(const std::string& app_id) {
  wl_output* output =
      connection_->wayland_output_manager()->GetPrimaryOutput()->output();
  agl_shell_activate_app(agl_shell_.get(), app_id.c_str(), output);
}

void AglShellWrapper::SetAglPanel(WaylandWindow* window, uint32_t edge) {
  wl_surface* surface = window->root_surface()->surface();
  wl_output* output =
      connection_->wayland_output_manager()->GetPrimaryOutput()->output();

  agl_shell_set_panel(agl_shell_.get(), surface, output, edge);
}

void AglShellWrapper::SetAglBackground(WaylandWindow* window) {
  wl_surface* surface = window->root_surface()->surface();
  wl_output* output =
      connection_->wayland_output_manager()->GetPrimaryOutput()->output();

  agl_shell_set_background(agl_shell_.get(), surface, output);
}

void AglShellWrapper::SetAglReady() {
  agl_shell_ready(agl_shell_.get());
}

}  // namespace ui