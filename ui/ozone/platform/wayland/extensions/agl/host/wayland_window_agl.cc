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

#include "ui/ozone/platform/wayland/extensions/agl/host/wayland_window_agl.h"

#include "base/logging.h"
#include "ui/ozone/platform/wayland/extensions/agl/host/agl_shell_wrapper.h"
#include "ui/ozone/platform/wayland/extensions/agl/host/wayland_extensions_agl.h"
#include "ui/ozone/platform/wayland/host/shell_surface_wrapper.h"
#include "ui/ozone/platform/wayland/host/wayland_connection.h"

namespace ui {

WaylandWindowAgl::WaylandWindowAgl(PlatformWindowDelegate* delegate,
                                   WaylandConnection* connection,
                                   WaylandExtensionsAgl* agl_extensions)
    : WaylandToplevelWindow(delegate, connection),
      agl_extensions_(agl_extensions) {}

WaylandWindowAgl::~WaylandWindowAgl() = default;

void WaylandWindowAgl::SetAglActivateApp(const std::string& app) {
  if (!agl_extensions_->GetAglShell()) {
    LOG(ERROR) << "Agl shell wrapper is not created";
    return;
  }

  agl_extensions_->GetAglShell()->SetAglActivateApp(app);
  connection()->Flush();
}

void WaylandWindowAgl::SetAglAppId(const std::string& title) {
  if (!shell_toplevel()) {
    LOG(ERROR) << "Shell toplevel is not created";
    return;
  }

  shell_toplevel()->SetAppId(title);
  connection()->Flush();
}

void WaylandWindowAgl::SetAglReady() {
  if (!agl_extensions_->GetAglShell()) {
    LOG(ERROR) << "Agl shell wrapper is not created";
    return;
  }

  agl_extensions_->GetAglShell()->SetAglReady();
  connection()->Flush();
}

void WaylandWindowAgl::SetAglBackground() {
  if (!agl_extensions_->GetAglShell()) {
    LOG(ERROR) << "Agl shell wrapper is not created";
    return;
  }

  agl_extensions_->GetAglShell()->SetAglBackground(this);
  connection()->Flush();
}

void WaylandWindowAgl::SetAglPanel(uint32_t edge) {
  if (!agl_extensions_->GetAglShell()) {
    LOG(ERROR) << "Agl shell wrapper is not created";
    return;
  }

  agl_extensions_->GetAglShell()->SetAglPanel(this, edge);
  connection()->Flush();
}

}  // namespace ui
