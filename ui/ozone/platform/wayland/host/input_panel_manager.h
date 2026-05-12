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

#ifndef UI_OZONE_PLATFORM_WAYLAND_HOST_INPUT_PANEL_MANAGER_H_
#define UI_OZONE_PLATFORM_WAYLAND_HOST_INPUT_PANEL_MANAGER_H_

namespace ui {

class InputPanel;
class WaylandWindow;

// The manager binding text input panels to active windows.
class InputPanelManager {
 public:
  virtual ~InputPanelManager() = default;

  // Returns text input panel bound to the |window|.
  virtual InputPanel* GetInputPanel(WaylandWindow* window) = 0;
};

}  // namespace ui

#endif  // UI_OZONE_PLATFORM_WAYLAND_HOST_INPUT_PANEL_MANAGER_H_
