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

#include "neva/app_runtime/ui/app_runtime_touch_menu_runner.h"

namespace neva_app_runtime {

AppRuntimeTouchSelectionMenuRunner::AppRuntimeTouchSelectionMenuRunner() {}
AppRuntimeTouchSelectionMenuRunner::~AppRuntimeTouchSelectionMenuRunner() {}

bool AppRuntimeTouchSelectionMenuRunner::IsMenuAvailable(
    const ui::TouchSelectionMenuClient* client,
    bool can_paste) const {
  return false;
}

void AppRuntimeTouchSelectionMenuRunner::OpenMenu(
    base::WeakPtr<ui::TouchSelectionMenuClient> client,
    const gfx::Rect& anchor_rect,
    const gfx::Size& handle_image_size,
    aura::Window* context,
    bool can_paste) {}

void AppRuntimeTouchSelectionMenuRunner::CloseMenu() {}

bool AppRuntimeTouchSelectionMenuRunner::IsRunning() const {
  return false;
}

}  // namespace neva_app_runtime
