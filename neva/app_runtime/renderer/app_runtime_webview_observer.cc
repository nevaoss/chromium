// Copyright 2026 LG Electronics, Inc.
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

#include "neva/app_runtime/renderer/app_runtime_webview_observer.h"

#include "third_party/blink/public/common/renderer_preferences/renderer_preferences.h"
#include "third_party/blink/public/platform/web_security_origin.h"
#include "third_party/blink/public/web/web_view.h"

namespace neva_app_runtime {

AppRuntimeWebViewObserver::AppRuntimeWebViewObserver(blink::WebView* web_view)
    : blink::WebViewObserver(web_view) {
  // Apply the preferences the WebView already has; updates arriving
  // later come through OnRendererPreferencesUpdated().
  OnRendererPreferencesUpdated(web_view->GetRendererPreferences());
}

void AppRuntimeWebViewObserver::OnDestruct() {
  delete this;
}

void AppRuntimeWebViewObserver::OnRendererPreferencesUpdated(
    const blink::RendererPreferences& preferences) {
  if (!preferences.file_security_origin.empty()) {
    url::Origin::SetFileOriginChanged(true);
  }

  blink::SetMutableLocalOrigin(preferences.file_security_origin);
}

}  // namespace neva_app_runtime
