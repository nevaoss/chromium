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

#ifndef NEVA_APP_RUNTIME_BROWSER_PERFORMANCE_MANAGER_WEBVIEW_MAP_H
#define NEVA_APP_RUNTIME_BROWSER_PERFORMANCE_MANAGER_WEBVIEW_MAP_H

#include <map>

#include "base/memory/singleton.h"
#include "content/public/browser/web_contents.h"
#include "neva/app_runtime/webview.h"

namespace contennt {
class WebContents;
}

namespace neva_app_runtime {

class WebViewMap final {
 public:
  static WebViewMap* GetInstance();

  void AddWebView(WebView* webview);
  void RemoveWebView(WebView* webview);
  void RemoveWebContents(content::WebContents* web_contents);
  WebView* FindWebViewFromWebContents(content::WebContents* web_contents);

 private:
  friend struct base::DefaultSingletonTraits<WebViewMap>;
  WebViewMap();
  ~WebViewMap();

  std::map<content::WebContents*, WebView*> webviews_;
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_BROWSER_PERFORMANCE_MANAGER_WEBVIEW_MAP_H
