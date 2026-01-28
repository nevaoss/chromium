// Copyright 2017 LG Electronics, Inc.
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

#ifndef UI_BASE_IME_NEVA_INPUT_METHOD_NEVA_OBSERVER_H_
#define UI_BASE_IME_NEVA_INPUT_METHOD_NEVA_OBSERVER_H_

#include <string>

#include "base/component_export.h"
#include "ui/base/ime/input_method_observer.h"

namespace ui {

struct TextInputInfo;

class COMPONENT_EXPORT(UI_BASE_IME) InputMethodNevaObserver
    : public InputMethodObserver {
 public:
  InputMethodNevaObserver();
  ~InputMethodNevaObserver() override;

  // InputMethodObserver overrides:
  void OnFocus() override;
  void OnBlur() override;
  void OnCaretBoundsChanged(const TextInputClient* client) override;
  void OnTextInputStateChanged(const TextInputClient* client) override;
  void OnInputMethodDestroyed(const InputMethod* input_method) override;
  void OnVirtualKeyboardVisibilityChangedIfEnabled(bool should_show) override;

  virtual void OnShowIme() = 0;
  virtual void OnHideIme() = 0;
  virtual void OnTextInputInfoChanged(const TextInputInfo& text_input_info) = 0;
  virtual void SetSurroundingText(const std::string& text,
                                  size_t cursor_position,
                                  size_t anchor_position) = 0;
  void SetImeEnabled(bool enable);
private:
  bool is_enabled_ = false;
};

}  // namespace ui

#endif  // UI_BASE_IME_NEVA_INPUT_METHOD_NEVA_OBSERVER_H_
