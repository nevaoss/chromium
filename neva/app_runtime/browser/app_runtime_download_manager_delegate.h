// Copyright 2022 LG Electronics, Inc.
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

#ifndef NEVA_APP_RUNTIME_BROWSER_APP_RUNTIME_DOWNLOAD_MANAGER_DELEGATE_H_
#define NEVA_APP_RUNTIME_BROWSER_APP_RUNTIME_DOWNLOAD_MANAGER_DELEGATE_H_

#include <memory>
#include <optional>

#include "content/public/browser/download_manager_delegate.h"

namespace pal {
class NotificationManagerDelegate;
}

namespace neva_app_runtime {

class AppRuntimeDownloadManagerDelegate
    : public content::DownloadManagerDelegate {
 public:
  AppRuntimeDownloadManagerDelegate();
  AppRuntimeDownloadManagerDelegate(const AppRuntimeDownloadManagerDelegate&) =
      delete;
  AppRuntimeDownloadManagerDelegate& operator=(
      const AppRuntimeDownloadManagerDelegate&) = delete;
  ~AppRuntimeDownloadManagerDelegate() override;

  void CheckDownloadAllowed(
      const content::WebContents::Getter& web_contents_getter,
      const GURL& url,
      const std::string& request_method,
      std::optional<url::Origin> request_initiator,
      bool from_download_cross_origin_redirect,
      bool content_initiated,
      const std::string& mime_type,
      std::optional<ui::PageTransition> page_transition,
      content::CheckDownloadAllowedCallback check_download_allowed_cb) override;

 private:
  std::unique_ptr<pal::NotificationManagerDelegate>
      notification_manager_delegate_;
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_BROWSER_APP_RUNTIME_DOWNLOAD_MANAGER_DELEGATE_H_
