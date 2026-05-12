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
//

#ifndef UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WEBOS_INPUT_PANEL_MANAGER_H_
#define UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WEBOS_INPUT_PANEL_MANAGER_H_

#include <map>
#include <memory>

#include "ui/ozone/platform/wayland/extensions/webos/host/webos_input_panel.h"
#include "ui/ozone/platform/wayland/host/input_panel_manager.h"

namespace ui {

class WaylandConnection;
class WaylandWindow;

class WebosInputPanelManager : public InputPanelManager {
 public:
  explicit WebosInputPanelManager(WaylandConnection* connection);
  WebosInputPanelManager(const WebosInputPanelManager&) = delete;
  WebosInputPanelManager& operator=(const WebosInputPanelManager&) = delete;
  ~WebosInputPanelManager() override;

  InputPanel* GetInputPanel(WaylandWindow* window) override;

 private:
  WaylandConnection* const connection_;

  std::map<WaylandWindow*, std::unique_ptr<WebosInputPanel>> input_panels_;
};

}  // namespace ui

#endif  // UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WEBOS_INPUT_PANEL_MANAGER_H_
