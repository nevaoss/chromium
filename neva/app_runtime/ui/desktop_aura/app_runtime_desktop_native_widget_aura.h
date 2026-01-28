// Copyright 2018 LG Electronics, Inc.
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

#ifndef NEVA_APP_RUNTIME_UI_DESKTOP_AURA_APP_RUNTIME_DESKTOP_NATIVE_WIDGET_AURA_H_
#define NEVA_APP_RUNTIME_UI_DESKTOP_AURA_APP_RUNTIME_DESKTOP_NATIVE_WIDGET_AURA_H_

#include "base/memory/raw_ptr.h"
#include "ui/views/widget/desktop_aura/desktop_native_widget_aura.h"
#include "ui/views/widget/native_widget_delegate.h"

namespace views {
class NativeEventDelegate;
}

namespace wm {
class ScopedTooltipDisabler;
}

namespace neva_app_runtime {

class WebAppWindow;

class AppRuntimeDesktopNativeWidgetAura : public views::DesktopNativeWidgetAura {
 public:
  explicit AppRuntimeDesktopNativeWidgetAura(
      views::internal::NativeWidgetDelegate* delegate);
  explicit AppRuntimeDesktopNativeWidgetAura(WebAppWindow* webapp_window);
  AppRuntimeDesktopNativeWidgetAura(const AppRuntimeDesktopNativeWidgetAura&) =
      delete;
  AppRuntimeDesktopNativeWidgetAura& operator=(
      const AppRuntimeDesktopNativeWidgetAura&) = delete;

  void OnWebAppWindowRemoved();
  void SetNativeEventDelegate(views::NativeEventDelegate*);
  // Overridden from views::DesktopNativeWidgetAura:
  views::NativeEventDelegate* GetNativeEventDelegate() const override;

 protected:
  ~AppRuntimeDesktopNativeWidgetAura() override;

 private:
  void InitNativeWidget(views::Widget::InitParams params) override;

  raw_ptr<WebAppWindow> webapp_window_ = nullptr;
  raw_ptr<views::NativeEventDelegate> native_event_delegate_ = nullptr;
  std::unique_ptr<wm::ScopedTooltipDisabler> tooltip_disabler_;
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_UI_DESKTOP_AURA_APP_RUNTIME_DESKTOP_NATIVE_WIDGET_AURA_H_
