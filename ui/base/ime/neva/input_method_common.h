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

#ifndef UI_BASE_IME_NEVA_INPUT_METHOD_COMMON_H_
#define UI_BASE_IME_NEVA_INPUT_METHOD_COMMON_H_

#include "base/component_export.h"
#include "ui/base/ime/neva/input_content_type.h"
#include "ui/base/ime/text_input_type.h"
#include "ui/gfx/geometry/rect.h"

namespace ui {

// The text input information for handling IME on the UI side.
struct COMPONENT_EXPORT(UI_BASE_IME) TextInputInfo {
  TextInputInfo() = default;
  ~TextInputInfo() = default;

  // Type of the input field.
  InputContentType type = InputContentType::kNone;

  // The input field flags (autocorrect, autocomplete, etc.)
  int flags = 0;

  // Position on the screen and dimensions of the input panel (virtual keyboard)
  // invoked for the input field.
  gfx::Rect input_panel_rectangle;
};

COMPONENT_EXPORT(UI_BASE_IME)
InputContentType GetInputContentTypeFromTextInputType(
    TextInputType text_input_type);

}  // namespace ui

#endif  // UI_BASE_IME_NEVA_INPUT_METHOD_COMMON_H_
