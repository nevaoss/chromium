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

#include "neva/app_runtime/webview_profile.h"

#include <memory>

#include "base/no_destructor.h"
#include "base/time/time.h"
#include "browser/browsing_data/browsing_data_remover.h"
#include "content/public/browser/browser_context.h"
#include "neva/app_runtime/app/app_runtime_main_delegate.h"
#include "neva/app_runtime/browser/app_runtime_browser_context.h"
#include "neva/app_runtime/public/proxy_settings.h"

namespace neva_app_runtime {

WebViewProfile::WebViewProfile(const std::string& storage_name)
    : browser_context_(AppRuntimeBrowserContext::From(storage_name)) {}

AppRuntimeBrowserContext* WebViewProfile::GetBrowserContext() const {
  return browser_context_;
}

WebViewProfile* WebViewProfile::GetDefaultProfile() {
  static base::NoDestructor<std::unique_ptr<WebViewProfile>> profile(
      new WebViewProfile(AppRuntimeBrowserContext::From("")));
  return (*profile).get();
}

WebViewProfile* WebViewProfile::GetAlternativeProfile() {
  static base::NoDestructor<std::unique_ptr<WebViewProfile>> profile(
      new WebViewProfile(AppRuntimeBrowserContext::From("private")));
  return (*profile).get();
}

void WebViewProfile::SetProxyServer(const ProxySettings& proxy_settings) {
  GetAppRuntimeContentBrowserClient()->SetProxyServer(proxy_settings);
}

void WebViewProfile::AppendExtraWebSocketHeader(const std::string& key,
                                                const std::string& value) {
  GetAppRuntimeContentBrowserClient()->AppendExtraWebSocketHeader(key, value);
}

void WebViewProfile::RemoveBrowsingData(int remove_browsing_data_mask) {
  BrowsingDataRemover* remover = BrowsingDataRemover::GetForStoragePartition(
      browser_context_->GetDefaultStoragePartition());
  remover->Remove(BrowsingDataRemover::Unbounded(), remove_browsing_data_mask);
}

void WebViewProfile::FlushCookieStore() {
  browser_context_->FlushCookieStore();
}

WebViewProfile::WebViewProfile(AppRuntimeBrowserContext* browser_context)
    : browser_context_(browser_context) {}

}  // namespace neva_app_runtime
