// Copyright 2016 LG Electronics, Inc.
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

#ifndef NEVA_APP_RUNTIME_PUBLIC_WEBAPP_WINDOW_BASE_H_
#define NEVA_APP_RUNTIME_PUBLIC_WEBAPP_WINDOW_BASE_H_

#include <memory>
#include <string>
#include <vector>

#include "neva/app_runtime/public/app_runtime_constants.h"
#include "neva/app_runtime/public/app_runtime_export.h"
#include "neva/app_runtime/public/webapp_window_delegate.h"
#include "ui/gfx/geometry/rect.h"

namespace neva_app_runtime {

class WebAppWindow;
class WindowGroupConfiguration;
using WebContents = void;

class APP_RUNTIME_EXPORT WebAppWindowBase : WebAppWindowDelegate {
 public:
  struct CreateParams {
    enum class WidgetType {
      kWindow,
      kWindowFrameless,
      kControl,
      kPopup,
      kMenu,
      kTooltip,
      kBubble,
      kDrag,
    };

    enum class WindowShowState {
      kDefault,
      kNormal,
      kMinimized,
      kMaximized,
      kInactive,
      kFullscreen,
      kEnd,
    };

    int width = 0;
    int height = 0;
    int pos_x = 0;
    int pos_y = 0;
    WidgetType type = WidgetType::kWindow;
    WindowShowState show_state = WindowShowState::kDefault;
    WebContents* web_contents;
  };

  struct Region {
    int pos_x;
    int pos_y;
    int width;
    int height;
  };

  WebAppWindowBase();
  WebAppWindowBase(const CreateParams& params);
  ~WebAppWindowBase() override;

  void Activate();
  void Show();
  void Minimize();
  void Close();

  void SetCustomCursor(CustomCursorType type,
                       const std::string& path,
                       int hotspot_x,
                       int hotspot_y);
  void SetHiddenState(bool hidden);
  void FirstFrameVisuallyCommitted();

  void AttachWebContents(WebContents* web_contents);
  void DetachWebContents();
  void RecreatedWebContents();

  WidgetState GetWindowHostState() const;
  WidgetState GetWindowHostStateAboutToChange() const;
  void SetWindowHostState(WidgetState state);
  // TODO(neva): Remove this to remove dependency of gfx
  void SetInputRegion(const std::vector<gfx::Rect>& region);
  void SetInputRegion(const std::vector<Region>& regions);
  void SetGroupKeyMask(KeyMask key_mask);
  void SetKeyMask(KeyMask key_mask, bool set);
  void SetWindowProperty(const std::string& name, const std::string& value);
  void SetOpacity(float opacity);
  void Resize(int width, int height);
  void SetBounds(int x, int y, int width, int height);
  bool IsKeyboardVisible() const;
  void SetUseVirtualKeyboard(bool enable);
  int GetWidth() const;
  int GetHeight() const;
  void CreateGroup(const WindowGroupConfiguration& config);
  void AttachToGroup(const std::string& group, const std::string& layer);
  void FocusGroupOwner();
  void FocusGroupLayer();
  void DetachGroup();

  void SetWindowTitle(const std::string& title);
  std::string GetWindowTitle() const;

  // Overridden from WebAppWindowDelegate
  void OnWindowClosing() override;

  void XInputActivate(const std::string& type = std::string());
  void XInputDeactivate();
  void XInputInvokeAction(
      uint32_t keysym,
      XInputKeySymbolType symType = XInputKeySymbolType::XINPUT_QT_KEY_SYMBOL,
      XInputEventType eventType = XInputEventType::XINPUT_PRESS_AND_RELEASE);

 private:
  WebAppWindow* webapp_window_;
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_PUBLIC_WEBAPP_WINDOW_BASE_H_
