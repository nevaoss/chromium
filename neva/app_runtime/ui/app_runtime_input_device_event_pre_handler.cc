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

#include "neva/app_runtime/ui/app_runtime_input_device_event_pre_handler.h"

#include "neva/app_runtime/webapp_window.h"

namespace neva_app_runtime {

AppRuntimeInputDeviceEventPreHandler::AppRuntimeInputDeviceEventPreHandler(
    WebAppWindow* webapp_window)
    : webapp_window_(webapp_window) {}

AppRuntimeInputDeviceEventPreHandler::~AppRuntimeInputDeviceEventPreHandler() {}

void AppRuntimeInputDeviceEventPreHandler::OnKeyEvent(ui::KeyEvent* event) {
  webapp_window_->OnKeyEvent(event);
}

void AppRuntimeInputDeviceEventPreHandler::OnMouseEvent(ui::MouseEvent* event) {
  webapp_window_->OnMouseEvent(event);
}

}  // namespace neva_app_runtime
