// Copyright 2018-2020 LG Electronics, Inc.
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

#ifndef WEBOS_WEBAPP_WINDOW_H_
#define WEBOS_WEBAPP_WINDOW_H_

#include "neva/app_runtime/public/webapp_window_base.h"
#include "neva/app_runtime/webapp_window.h"
#include "ui/views/widget/desktop_aura/neva/ui_constants.h"

namespace ui {
enum class DomCode : uint32_t;
}

class WebOSEvent;

namespace webos {

class WebAppWindowDelegate;

class WebAppWindow : public neva_app_runtime::WebAppWindow {
 public:
  WebAppWindow(const neva_app_runtime::WebAppWindowBase::CreateParams& params);
  WebAppWindow(const WebAppWindow&) = delete;
  WebAppWindow& operator=(const WebAppWindow&) = delete;
  ~WebAppWindow() override;

  void SetDelegate(WebAppWindowDelegate* webapp_window_delegate);
  bool HandleEvent(WebOSEvent* webos_event);

  void ActivateAndShow();

  // Overridden from views::NativeEventDelegate
  void CursorVisibilityChanged(bool visible) override;
  void InputPanelVisibilityChanged(bool visibility) override;
  void InputPanelRectChanged(int32_t x,
                             int32_t y,
                             uint32_t width,
                             uint32_t height) override;
  void KeyboardEnter() override;
  void KeyboardLeave() override;
  void WindowHostClose() override;
  void WindowHostExposed() override;
  void WindowHostStateChanged(ui::WidgetState new_state) override;
  void WindowHostStateAboutToChange(ui::WidgetState state) override;

  // Overridden from ui::EventHandler
  void OnMouseEvent(ui::MouseEvent* event) override;
  void OnKeyEvent(ui::KeyEvent* event) override;

 private:
  // Handle key events
  bool OnKeyPressed(unsigned keycode);
  bool OnKeyReleased(unsigned keycode);

  void CheckKeyFilter(ui::KeyEvent* event);
  void CheckKeyFilterTable(unsigned keycode,
                           unsigned& new_keycode,
                           ui::DomCode& code,
                           unsigned& modifier);

  WebAppWindowDelegate* webapp_window_delegate_ = nullptr;
  bool keyboard_enter_ = false;
};

}  // namespace webos

#endif  // WEBOS_WEBAPP_WINDOW_H_
