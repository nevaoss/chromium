// Copyright 2017 LG Electronics, Inc.
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

#ifndef UI_PLATFORM_WINDOW_NEVA_PLATFORM_WINDOW_DELEGATE_H_
#define UI_PLATFORM_WINDOW_NEVA_PLATFORM_WINDOW_DELEGATE_H_

#include <string>
#include <vector>

#include "ui/views/widget/desktop_aura/neva/ui_constants.h"

namespace ui {

enum class PlatformWindowState;

namespace neva {

// ozone-wayland additions for platform window delegate.
class PlatformWindowDelegate {
 public:
  virtual void OnDragEnter(unsigned windowhandle,
                           float x,
                           float y,
                           const std::vector<std::string>& mime_types,
                           std::uint32_t serial) {}

  virtual void OnDragDataReceived(int fd) {}

  virtual void OnDragLeave() {}

  virtual void OnDragMotion(float x, float y, std::uint32_t time) {}

  virtual void OnDragDrop() {}

  virtual void OnInputPanelVisibilityChanged(bool visible) {}
  virtual void OnInputPanelRectChanged(std::int32_t x,
                                       std::int32_t y,
                                       std::uint32_t width,
                                       std::uint32_t height) {}
  virtual void OnWindowHostExposed() {}
  virtual void OnWindowHostClose() {}
  virtual void OnKeyboardEnter() {}
  virtual void OnKeyboardLeave() {}
  virtual void OnWindowHostStateChanged(ui::WidgetState new_state) {}
  virtual void OnWindowHostStateAboutToChange(ui::WidgetState state) {}
  virtual void OnWindowStateAboutToChange(ui::PlatformWindowState state) {}
  virtual void OnCursorVisibilityChanged(bool visible) {}
};

}  // namespace neva
}  // namespace ui

#endif  // UI_PLATFORM_WINDOW_NEVA_PLATFORM_WINDOW_DELEGATE_H_
