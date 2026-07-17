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

#ifndef NEVA_APP_RUNTIME_BROWSER_APP_RUNTIME_BROWSER_CONTEXT_H_
#define NEVA_APP_RUNTIME_BROWSER_APP_RUNTIME_BROWSER_CONTEXT_H_

#include <map>
#include <string_view>

#include "base/files/file_path.h"
#include "content/public/browser/browser_context.h"

class PrefService;

namespace neva_app_runtime {

class AppRuntimeDownloadManagerDelegate;
struct StoragePartitionInfo;

class AppRuntimeBrowserContext : public content::BrowserContext {
 public:
  static AppRuntimeBrowserContext* From(std::string_view partition,
                                        bool off_the_record = false);
  static AppRuntimeBrowserContext* From(const StoragePartitionInfo& info);

  static void Clear();

  AppRuntimeBrowserContext(const AppRuntimeBrowserContext&) = delete;
  AppRuntimeBrowserContext& operator=(const AppRuntimeBrowserContext&) = delete;
  ~AppRuntimeBrowserContext() override;
  base::FilePath GetPath() const override;
  bool IsOffTheRecord() override;

  content::DownloadManagerDelegate* GetDownloadManagerDelegate() override;
  content::BrowserPluginGuestManager* GetGuestManager() override;
  storage::SpecialStoragePolicy* GetSpecialStoragePolicy() override;
  content::PlatformNotificationService* GetPlatformNotificationService()
      override;
  content::PushMessagingService* GetPushMessagingService() override;
  content::StorageNotificationService* GetStorageNotificationService() override;
  content::SSLHostStateDelegate* GetSSLHostStateDelegate() override;
  std::unique_ptr<content::ZoomLevelDelegate> CreateZoomLevelDelegate(
      const base::FilePath&) override;
  content::PermissionControllerDelegate* GetPermissionControllerDelegate()
      override;
  content::ReduceAcceptLanguageControllerDelegate*
  GetReduceAcceptLanguageControllerDelegate() override;
  content::ClientHintsControllerDelegate* GetClientHintsControllerDelegate()
      override;
  content::BackgroundFetchDelegate* GetBackgroundFetchDelegate() override;
  content::BackgroundSyncController* GetBackgroundSyncController() override;
  content::BrowsingDataRemoverDelegate* GetBrowsingDataRemoverDelegate()
      override;

  bool ExtensionsIsAllowed();
  void FlushCookieStore();
  std::string GetName() const;

 private:
  AppRuntimeBrowserContext(std::string_view partition,
                           bool off_the_record,
                           bool allow_to_load_extensions);
  base::FilePath InitPath(std::string_view partition) const;

  using BrowserContextMap = std::map<std::string,
                                     std::unique_ptr<AppRuntimeBrowserContext>,
                                     std::less<>>;
  static BrowserContextMap& browser_context_map();
  static BrowserContextMap& off_the_record_browser_context_map();

  bool off_the_record_;
  bool extensions_is_allowed_;
  std::string name_;

  std::unique_ptr<AppRuntimeDownloadManagerDelegate> download_manager_delegate_;
  std::unique_ptr<content::ClientHintsControllerDelegate> hints_delegate_;
  std::unique_ptr<PrefService> user_pref_service_;
  const base::FilePath path_;

#if defined(USE_NEVA_CHROME_EXTENSIONS)
  std::unique_ptr<PrefService> local_state_;
#endif  // defined(USE_NEVA_CHROME_EXTENSIONS)
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_BROWSER_APP_RUNTIME_BROWSER_CONTEXT_H_
