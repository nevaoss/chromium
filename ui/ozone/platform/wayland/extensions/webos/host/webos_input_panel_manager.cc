// Copyright 2020 LG Electronics, Inc.
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

#include "ui/ozone/platform/wayland/extensions/webos/host/webos_input_panel_manager.h"

#include "ui/ozone/platform/wayland/extensions/webos/host/wayland_window_webos.h"

namespace ui {

WebosInputPanelManager::WebosInputPanelManager(WaylandConnection* connection)
    : connection_(connection) {}

WebosInputPanelManager::~WebosInputPanelManager() = default;

InputPanel* WebosInputPanelManager::GetInputPanel(WaylandWindow* window) {
  if (!window)
    return nullptr;

  if (input_panels_.find(window) == input_panels_.end()) {
    input_panels_[window] = std::make_unique<WebosInputPanel>(
        connection_, static_cast<WaylandWindowWebos*>(window));
  }

  return input_panels_[window].get();
}

}  // namespace ui
