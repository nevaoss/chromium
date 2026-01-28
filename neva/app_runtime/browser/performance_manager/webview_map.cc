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

#include "neva/app_runtime/browser/performance_manager/webview_map.h"

namespace neva_app_runtime {

WebViewMap::WebViewMap() = default;

WebViewMap::~WebViewMap() = default;

WebViewMap* WebViewMap::GetInstance() {
  return base::Singleton<WebViewMap>::get();
}

void WebViewMap::AddWebView(WebView* webview) {
  if (webview && webview->GetWebContents()) {
    webviews_[webview->GetWebContents()] = webview;
  }
}

void WebViewMap::RemoveWebView(WebView* webview) {
  if (webview) {
    webviews_.erase(webview->GetWebContents());
  }
}

void WebViewMap::RemoveWebContents(content::WebContents* web_contents) {
  webviews_.erase(web_contents);
}

WebView* WebViewMap::FindWebViewFromWebContents(
    content::WebContents* web_contents) {
  auto it = webviews_.find(web_contents);
  if (it != webviews_.end()) {
    return it->second;
  }
  return nullptr;
}

}  // namespace neva_app_runtime
