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

#include "ui/aura/neva/agl/window_tree_host_platform_agl.h"

#include "ui/aura/window_tree_host_platform.h"
#include "ui/platform_window/platform_window.h"

namespace aura {
namespace neva {

WindowTreeHostPlatformAgl::WindowTreeHostPlatformAgl(
    std::unique_ptr<Window> window,
    aura::WindowTreeHostPlatform* window_tree_host_platform)
    : aura::WindowTreeHost(std::move(window)),
      window_tree_host_platform_(window_tree_host_platform) {}

void WindowTreeHostPlatformAgl::SetAglActivateApp(const std::string& app) {
  window_tree_host_platform_->platform_window()->SetAglActivateApp(app);
}

void WindowTreeHostPlatformAgl::SetAglAppId(const std::string& title) {
  window_tree_host_platform_->platform_window()->SetAglAppId(title);
}

void WindowTreeHostPlatformAgl::SetAglReady() {
  window_tree_host_platform_->platform_window()->SetAglReady();
}

void WindowTreeHostPlatformAgl::SetAglBackground() {
  window_tree_host_platform_->platform_window()->SetAglBackground();
}

void WindowTreeHostPlatformAgl::SetAglPanel(uint32_t edge) {
  window_tree_host_platform_->platform_window()->SetAglPanel(edge);
}

}  // namespace neva
}  // namespace aura