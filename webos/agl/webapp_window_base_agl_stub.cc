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

#include "webos/agl/webapp_window_base_agl.h"

#include "webos/webapp_window_base.h"

namespace webos {
namespace agl {

WebAppWindowBaseAgl::WebAppWindowBaseAgl(
    WebAppWindowBase* webapp_window_base) {}

void WebAppWindowBaseAgl::InitWindow() {}

void WebAppWindowBaseAgl::SetAglActivateApp(const char* app_id) {}

void WebAppWindowBaseAgl::SetAglAppId(const char* app_id) {}

void WebAppWindowBaseAgl::SetAglBackground() {}

void WebAppWindowBaseAgl::SetAglReady() {}

void WebAppWindowBaseAgl::SetAglPanel(int edge) {}

void WebAppWindowBaseAgl::SetWindowSurfaceId(int surface_id) {}

}  // namespace agl
}  // namespace webos