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

#include "neva/app_runtime/browser/ui/app_runtime_views_delegate.h"

#include "neva/app_runtime/ui/desktop_aura/app_runtime_desktop_native_widget_aura.h"
#include "ui/views/widget/widget.h"

namespace neva_app_runtime {

AppRuntimeViewsDelegate::AppRuntimeViewsDelegate() {
}

AppRuntimeViewsDelegate::~AppRuntimeViewsDelegate() {
}

void AppRuntimeViewsDelegate::OnBeforeWidgetInit(
    views::Widget::InitParams* params,
    views::internal::NativeWidgetDelegate* delegate) {
  params->native_widget = CreateNativeWidget(params, delegate);
}

views::NativeWidget* AppRuntimeViewsDelegate::CreateNativeWidget(
    views::Widget::InitParams* params,
    views::internal::NativeWidgetDelegate* delegate) {
  views::DesktopNativeWidgetAura* desktop_native_widget =
      new AppRuntimeDesktopNativeWidgetAura(delegate);
  return desktop_native_widget;
}

}  // namespace neva_app_runtime
