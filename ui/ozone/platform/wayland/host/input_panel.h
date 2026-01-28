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

#ifndef UI_OZONE_PLATFORM_WAYLAND_HOST_INPUT_PANEL_H_
#define UI_OZONE_PLATFORM_WAYLAND_HOST_INPUT_PANEL_H_

#include <cstdint>
#include <string>

namespace ui {

struct TextInputInfo;

// The text input panel associated with an active window.
class InputPanel {
 public:
  virtual ~InputPanel() = default;

  // Hides the text input panel (virtual keyboard).
  virtual void HideInputPanel() = 0;

  // Sets |text_input_info| (content type, flags, etc.)
  // for the text input panel (virtual keyboard).
  virtual void SetTextInputInfo(const TextInputInfo& text_input_info) = 0;

  // Sets the surrounding |text| around the input position.
  virtual void SetSurroundingText(const std::string& text,
                                  std::size_t cursor_position,
                                  std::size_t anchor_position) = 0;

  // Shows the text input panel (virtual keyboard).
  virtual void ShowInputPanel() = 0;
};

}  // namespace ui

#endif  // UI_OZONE_PLATFORM_WAYLAND_HOST_INPUT_PANEL_H_
