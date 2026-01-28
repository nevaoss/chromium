// Copyright 2018 LG Electronics, Inc.
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

#include "webos/webapp_window.h"

#include "ui/events/keycodes/dom/dom_code.h"
#include "webos/common/webos_event.h"
#include "webos/public/runtime.h"
#include "webos/webapp_window_delegate.h"

namespace webos {

WebAppWindow::WebAppWindow(
    const neva_app_runtime::WebAppWindowBase::CreateParams& params)
    : neva_app_runtime::WebAppWindow(params, nullptr) {
  SetDeferredDeleting(true);
}

WebAppWindow::~WebAppWindow() {
  if (webapp_window_delegate_)
    webapp_window_delegate_->WebAppWindowDestroyed();
}

void WebAppWindow::SetDelegate(WebAppWindowDelegate* webapp_window_delegate) {
  webapp_window_delegate_ = webapp_window_delegate;
}

bool WebAppWindow::HandleEvent(WebOSEvent* webos_event) {
  if (webapp_window_delegate_)
    return webapp_window_delegate_->HandleWebOSEvent(webos_event);

  return false;
}

void WebAppWindow::CursorVisibilityChanged(bool visible) {
  neva_app_runtime::WebAppWindow::CursorVisibilityChanged(visible);
  Runtime::GetInstance()->OnCursorVisibilityChanged(visible);
}

void WebAppWindow::InputPanelVisibilityChanged(bool visibility) {
  neva_app_runtime::WebAppWindow::InputPanelVisibilityChanged(visibility);
  WebOSVirtualKeyboardEvent webos_event(WebOSEvent::Type::InputPanelVisible,
                                        visibility,
                                        visibility ? input_panel_height() : 0);
  HandleEvent(&webos_event);
}

void WebAppWindow::InputPanelRectChanged(int32_t x,
                                         int32_t y,
                                         uint32_t width,
                                         uint32_t height) {
  neva_app_runtime::WebAppWindow::InputPanelRectChanged(x, y, width, height);
}

void WebAppWindow::KeyboardEnter() {
  if (keyboard_enter_)
    return;

  keyboard_enter_ = true;

  neva_app_runtime::WebAppWindow::KeyboardEnter();

  WebOSEvent webos_event(WebOSEvent::Type::FocusIn);
  HandleEvent(&webos_event);
}

void WebAppWindow::KeyboardLeave() {
  if (!keyboard_enter_)
    return;

  keyboard_enter_ = false;

  neva_app_runtime::WebAppWindow::KeyboardLeave();

  WebOSEvent webos_event(WebOSEvent::Type::FocusOut);
  HandleEvent(&webos_event);
}

void WebAppWindow::WindowHostClose() {
  neva_app_runtime::WebAppWindow::WindowHostClose();

  WebOSEvent webos_event(WebOSEvent::Close);
  HandleEvent(&webos_event);
}

void WebAppWindow::WindowHostExposed() {
  neva_app_runtime::WebAppWindow::WindowHostExposed();

  WebOSEvent webos_event(WebOSEvent::Type::Expose);
  HandleEvent(&webos_event);
}

void WebAppWindow::WindowHostStateChanged(ui::WidgetState new_state) {
  neva_app_runtime::WebAppWindow::WindowHostStateChanged(new_state);

  WebOSEvent webos_event(WebOSEvent::Type::WindowStateChange);
  HandleEvent(&webos_event);
}

void WebAppWindow::WindowHostStateAboutToChange(ui::WidgetState state) {
  neva_app_runtime::WebAppWindow::WindowHostStateAboutToChange(state);

  WebOSEvent webos_event(WebOSEvent::Type::WindowStateAboutToChange);
  HandleEvent(&webos_event);
}

void WebAppWindow::OnMouseEvent(ui::MouseEvent* event) {
  neva_app_runtime::WebAppWindow::OnMouseEvent(event);

  float x = event->x();
  float y = event->y();
  int flags = event->flags();

  switch (event->type()) {
    case ui::EventType::kMousePressed: {
      WebOSMouseEvent ev(WebOSEvent::MouseButtonPress, x, y, flags);
      if (HandleEvent(&ev))
        event->StopPropagation();
      break;
    }
    case ui::EventType::kMouseReleased: {
      WebOSMouseEvent ev(WebOSEvent::Type::MouseButtonRelease, x, y, flags);
      if (HandleEvent(&ev))
        event->StopPropagation();
      break;
    }
    case ui::EventType::kMouseMoved: {
      WebOSMouseEvent ev(WebOSEvent::Type::MouseMove, x, y);
      if (HandleEvent(&ev))
        event->StopPropagation();
      break;
    }
    case ui::EventType::kMouseEntered: {
      WebOSMouseEvent ev(WebOSEvent::Type::Enter, x, y);
      HandleEvent(&ev);
      break;
    }
    case ui::EventType::kMouseExited: {
      WebOSMouseEvent ev(WebOSEvent::Type::Leave, x, y);
      HandleEvent(&ev);
      break;
    }
    case ui::EventType::kMousewheel: {
      ui::MouseWheelEvent* wheel_event =
          static_cast<ui::MouseWheelEvent*>(event);
      WebOSMouseWheelEvent ev(WebOSEvent::Type::Wheel, x, y,
                              wheel_event->x_offset(), wheel_event->y_offset());
      if (HandleEvent(&ev))
        event->StopPropagation();
      break;
    }
    default:
      break;
  }
}

void WebAppWindow::OnKeyEvent(ui::KeyEvent* event) {
  CheckKeyFilter(event);

  switch (event->type()) {
    case ui::EventType::kKeyPressed:
      if (OnKeyPressed(event->key_code()))
        event->StopPropagation();
      break;
    case ui::EventType::kKeyReleased:
      if (OnKeyReleased(event->key_code()))
        event->StopPropagation();
      break;
    default:
      break;
  }
}

void WebAppWindow::CheckKeyFilter(ui::KeyEvent* event) {
  unsigned new_keycode;
  ui::DomCode code;
  unsigned modifier = 0;
  CheckKeyFilterTable(event->key_code(), new_keycode, code, modifier);
  if (new_keycode) {
    event->set_key_code(static_cast<ui::KeyboardCode>(new_keycode));
    event->set_code(code);
  }
}

void WebAppWindow::CheckKeyFilterTable(unsigned keycode,
                                       unsigned& new_keycode,
                                       ui::DomCode& code,
                                       unsigned& modifier) {
  new_keycode = 0;
  code = ui::DomCode::NONE;
  if (webapp_window_delegate_) {
    unsigned filtered_keycode =
        webapp_window_delegate_->CheckKeyFilterTable(keycode, &modifier);
  }
}

bool WebAppWindow::OnKeyPressed(unsigned keycode) {
  WebOSKeyEvent event(WebOSEvent::Type::KeyPress, keycode);
  return HandleEvent(&event);
}

bool WebAppWindow::OnKeyReleased(unsigned keycode) {
  WebOSKeyEvent event(WebOSKeyEvent::Type::KeyRelease, keycode);
  return HandleEvent(&event);
}

void WebAppWindow::ActivateAndShow() {
  RecreateIfNeeded();
  Activate();
  Show();
}

}  // namespace webos
