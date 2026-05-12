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

#include "neva/app_runtime/browser/app_runtime_download_manager_delegate.h"

#include <optional>

#include "base/command_line.h"
#include "base/neva/base_switches.h"
#include "base/task/single_thread_task_runner.h"
#include "neva/pal_service/pal_platform_factory.h"
#include "neva/pal_service/public/notification_manager_delegate.h"
#include "third_party/blink/public/mojom/renderer_preferences.mojom.h"

namespace neva_app_runtime {

AppRuntimeDownloadManagerDelegate::AppRuntimeDownloadManagerDelegate() {
  if (base::CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kEnableNotificationForUnsupportedFeatures)) {
    notification_manager_delegate_ =
        pal::PlatformFactory::Get()->CreateNotificationManagerDelegate();
  }
}

AppRuntimeDownloadManagerDelegate::~AppRuntimeDownloadManagerDelegate() =
    default;

void AppRuntimeDownloadManagerDelegate::CheckDownloadAllowed(
    const content::WebContents::Getter& web_contents_getter,
    const GURL& url,
    const std::string& request_method,
    std::optional<url::Origin> request_initiator,
    bool from_download_cross_origin_redirect,
    bool content_initiated,
    const std::string& mime_type,
    std::optional<ui::PageTransition> page_transition,
    content::CheckDownloadAllowedCallback check_download_allowed_cb) {
  if (notification_manager_delegate_) {
    content::WebContents* web_contents = web_contents_getter.Run();
    if (web_contents) {
      notification_manager_delegate_->CreateToast(
          web_contents->GetMutableRendererPrefs()->application_id,
          "Download is not supported.");
    }
  }
  base::SingleThreadTaskRunner::GetCurrentDefault()->PostTask(
      FROM_HERE, base::BindOnce(std::move(check_download_allowed_cb), false));
}

}  // namespace neva_app_runtime
