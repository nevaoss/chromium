// Copyright 2017-2019 LG Electronics, Inc.
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

#ifndef NEVA_APP_RUNTIME_WEBVIEW_PROFILE_H_
#define NEVA_APP_RUNTIME_WEBVIEW_PROFILE_H_

#include <string>

#include "base/memory/raw_ptr.h"
#include "neva/app_runtime/public/app_runtime_export.h"

namespace neva_app_runtime {

class AppRuntimeBrowserContext;
struct ProxySettings;

class APP_RUNTIME_EXPORT WebViewProfile {
 public:
  WebViewProfile(const std::string& storage_name);

  AppRuntimeBrowserContext* GetBrowserContext() const;

  static WebViewProfile* GetDefaultProfile();
  static WebViewProfile* GetAlternativeProfile();

  void SetProxyServer(const ProxySettings& proxy_settings);
  void AppendExtraWebSocketHeader(const std::string& key,
                                  const std::string& value);

  void RemoveBrowsingData(int remove_browsing_data_mask);

  void FlushCookieStore();

 private:
  WebViewProfile(AppRuntimeBrowserContext* browser_context);

  friend class WebView;
  raw_ptr<AppRuntimeBrowserContext> browser_context_;
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_WEBVIEW_PROFILE_H_
