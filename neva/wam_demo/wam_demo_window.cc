// Copyright 2017-2020 LG Electronics, Inc.
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

#include "neva/wam_demo/wam_demo_window.h"

#include "base/logging.h"
#include "neva/wam_demo/wam_demo_window_observer.h"

namespace wam_demo {

WamDemoWindow::WamDemoWindow(
    const neva_app_runtime::WebAppWindowBase::CreateParams& params,
    WamDemoWindowObserver* observer)
    : neva_app_runtime::WebAppWindowBase(params),
      observer_(observer) {}

void WamDemoWindow::OnWindowClosing() {
  observer_->OnWindowClosing(this);
}

void WamDemoWindow::CursorVisibilityChanged(bool visible) {
  observer_->CursorVisibilityChanged(this, visible);
}

bool WamDemoWindow::HandleEvent(neva_app_runtime::AppRuntimeEvent* e) {
  switch (e->GetType()) {
    case neva_app_runtime::AppRuntimeEvent::Close:
      LOG(INFO) << __func__ << ": Close event is handled";
      break;
    case neva_app_runtime::AppRuntimeEvent::Expose:
      LOG(INFO) << __func__ << ": Expose event is handled";
      break;
    case neva_app_runtime::AppRuntimeEvent::FocusIn:
      LOG(INFO) << __func__ << ": FocusIn event is handled";
      break;
    case neva_app_runtime::AppRuntimeEvent::FocusOut:
      LOG(INFO) << __func__ << ": FocusOut event is handled";
      break;
    case neva_app_runtime::AppRuntimeEvent::WindowStateChange:
      LOG(INFO) << __func__ << ": WindowStateChange event is handled";
      break;
    case neva_app_runtime::AppRuntimeEvent::WindowStateAboutToChange:
      LOG(INFO) << __func__ << ": WindowStateAboutToChange event is handled";
      break;
    default:
      break;
  }
  return true;
}

}  // namespace wam_demo
