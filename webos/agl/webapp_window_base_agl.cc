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

#include "webos/webapp_window.h"
#include "webos/webapp_window_base.h"

namespace webos {
namespace agl {

WebAppWindowBaseAgl::WebAppWindowBaseAgl(WebAppWindowBase* webapp_window_base)
    : webapp_window_base_(webapp_window_base) {}

void WebAppWindowBaseAgl::InitWindow() {
  // Perform calls which were unavailable before due to uninitialization
  if (pending_agl_background_)
    SetAglBackground();

  if (pending_agl_edge_ != -1)
    SetAglPanel(pending_agl_edge_);

  if (pending_agl_ready_)
    SetAglReady();
}

void WebAppWindowBaseAgl::SetAglActivateApp(const char* app_id) {
  auto webapp_window = webapp_window_base_->GetWebAppWindow();
  if (webapp_window)
    webapp_window->SetAglActivateApp(app_id);
}

void WebAppWindowBaseAgl::SetAglAppId(const char* app_id) {
  auto webapp_window = webapp_window_base_->GetWebAppWindow();
  if (webapp_window) {
    app_id_ = std::string(app_id);
    webapp_window->SetAglAppId(app_id_);
  }
}

void WebAppWindowBaseAgl::SetAglBackground() {
  auto webapp_window = webapp_window_base_->GetWebAppWindow();
  if (!webapp_window)
    pending_agl_background_ = true;
  else
    webapp_window->SetAglBackground();
}

void WebAppWindowBaseAgl::SetAglReady() {
  auto webapp_window = webapp_window_base_->GetWebAppWindow();
  if (!webapp_window)
    pending_agl_ready_ = true;
  else
    webapp_window->SetAglReady();
}

void WebAppWindowBaseAgl::SetAglPanel(int edge) {
  auto webapp_window = webapp_window_base_->GetWebAppWindow();
  if (!webapp_window)
    pending_agl_edge_ = edge;
  else
    webapp_window->SetAglPanel(edge);
}

void WebAppWindowBaseAgl::SetWindowSurfaceId(int surface_id) {}

}  // namespace agl
}  // namespace webos