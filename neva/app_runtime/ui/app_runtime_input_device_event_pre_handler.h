// Copyright 2024 LG Electronics, Inc.
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

#ifndef NEVA_APP_RUNTIME_UI_APP_RUNTIME_INPUT_DEVICE_EVENT_PRE_HANDLER_H_
#define NEVA_APP_RUNTIME_UI_APP_RUNTIME_INPUT_DEVICE_EVENT_PRE_HANDLER_H_

#include "base/memory/raw_ptr.h"
#include "ui/events/event_handler.h"

namespace neva_app_runtime {

class WebAppWindow;

class AppRuntimeInputDeviceEventPreHandler : public ui::EventHandler {
 public:
  explicit AppRuntimeInputDeviceEventPreHandler(WebAppWindow* delegate);

  AppRuntimeInputDeviceEventPreHandler(
      const AppRuntimeInputDeviceEventPreHandler&) = delete;

  AppRuntimeInputDeviceEventPreHandler& operator=(
      const AppRuntimeInputDeviceEventPreHandler&) = delete;

  ~AppRuntimeInputDeviceEventPreHandler() override;

  void OnMouseEvent(ui::MouseEvent* event) override;

  void OnKeyEvent(ui::KeyEvent* event) override;

 private:
  raw_ptr<WebAppWindow> webapp_window_;
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_UI_APP_RUNTIME_INPUT_DEVICE_EVENT_PRE_HANDLER_H_
