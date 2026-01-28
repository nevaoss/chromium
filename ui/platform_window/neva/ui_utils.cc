// Copyright 2019 LG Electronics, Inc.
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

#include "ui/platform_window/neva/ui_utils.h"

#include "ui/platform_window/platform_window_delegate.h"
#include "ui/views/widget/desktop_aura/neva/ui_constants.h"

namespace ui {

WidgetState ToWidgetState(PlatformWindowState state) {
  switch (state) {
    case PlatformWindowState::kMinimized:
      return WidgetState::MINIMIZED;
    case PlatformWindowState::kMaximized:
      return WidgetState::MAXIMIZED;
    case PlatformWindowState::kFullScreen:
      return WidgetState::FULLSCREEN;
    default:
      return WidgetState::UNINITIALIZED;
  }
}

}  // namespace ui
