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

#ifndef NEVA_WAM_DEMO_WAM_DEMO_WINDOW_H_
#define NEVA_WAM_DEMO_WAM_DEMO_WINDOW_H_

#include "base/memory/raw_ptr.h"
#include "neva/app_runtime/public/app_runtime_constants.h"
#include "neva/app_runtime/public/app_runtime_event.h"
#include "neva/app_runtime/public/webapp_window_base.h"

namespace wam_demo {

class WamDemoWindowObserver;

class WamDemoWindow : public neva_app_runtime::WebAppWindowBase {
 public:
  WamDemoWindow(const neva_app_runtime::WebAppWindowBase::CreateParams& params,
                WamDemoWindowObserver* observer);
  WamDemoWindow(const WamDemoWindow&) = delete;
  WamDemoWindow& operator = (const WamDemoWindow&) = delete;

  void OnWindowClosing() override;
  void CursorVisibilityChanged(bool visible) override;
  bool HandleEvent(neva_app_runtime::AppRuntimeEvent* e) override;

 private:
  raw_ptr<WamDemoWindowObserver> observer_;
};

}  // namespace wam_demo

#endif  // NEVA_WAM_DEMO_WAM_DEMO_WINDOW_H_
