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

#include "neva/app_runtime/browser/app_runtime_browser_context.h"

#include "base/base_paths_posix.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/functional/bind.h"
#include "base/logging.h"
#include "base/no_destructor.h"
#include "base/path_service.h"
#include "base/strings/escape.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/common/content_switches.h"
#include "net/base/http_user_agent_settings.h"
#include "net/cookies/cookie_store.h"
#include "net/http/http_network_session.h"
#include "net/http/http_request_headers.h"
#include "net/proxy_resolution/proxy_config_service.h"
#include "net/proxy_resolution/proxy_info.h"
#include "net/proxy_resolution/proxy_resolution_service.h"
#include "neva/app_runtime/browser/app_runtime_browser_switches.h"
#include "neva/app_runtime/browser/app_runtime_download_manager_delegate.h"
#include "neva/app_runtime/browser/app_runtime_prefs.h"
#include "neva/app_runtime/browser/app_runtime_storage_partition_name.h"
#include "neva/app_runtime/browser/browsing_data/browsing_data_remover.h"
#include "neva/app_runtime/browser/permissions/app_runtime_permission_manager_factory.h"
#include "neva/user_agent/browser/client_hints.h"
#include "services/network/public/mojom/cookie_manager.mojom.h"

#if defined(USE_NEVA_CHROME_EXTENSIONS)
#include "components/sessions/core/session_id_generator.h"
#include "extensions/browser/api/api_browser_context_keyed_service_factories.h"
#include "extensions/browser/browser_context_keyed_service_factories.h"
#include "neva/app_runtime/browser/extensions/tab_helper_impl.h"
#include "neva/extensions/browser/browser_context_keyed_service_factories.h"
#include "neva/extensions/browser/neva_extension_system.h"
#include "neva/extensions/browser/neva_extensions_browser_client.h"
#include "neva/extensions/browser/neva_extensions_service_factory.h"
#include "neva/extensions/browser/neva_extensions_service_impl.h"
#include "neva/extensions/browser/neva_extensions_util.h"
#include "neva/extensions/common/neva_extensions_client.h"
#endif  // defined(USE_NEVA_CHROME_EXTENSIONS)

namespace neva_app_runtime {

bool IsAllowedToLoadExtensionsIn(std::string partition, bool off_the_record) {
  base::CommandLine* cmd_line = base::CommandLine::ForCurrentProcess();
  if (cmd_line->HasSwitch(kAllowLoadExtensionsIn)) {
    std::vector<std::string> allowed_list =
        base::SplitString(cmd_line->GetSwitchValueASCII(kAllowLoadExtensionsIn),
                          ",", base::TRIM_WHITESPACE, base::SPLIT_WANT_ALL);

    for (auto& it : allowed_list) {
      StoragePartitionInfo allowed_item = ParseStoragePartitionName(it);
      if ((partition == allowed_item.name) &&
          (off_the_record == allowed_item.off_the_record)) {
        return true;
      }
    }

    return false;
  }

  return true;
}

// static
AppRuntimeBrowserContext* AppRuntimeBrowserContext::From(std::string partition,
                                                         bool off_the_record) {
  bool allow_to_load_extensions =
      IsAllowedToLoadExtensionsIn(partition, off_the_record);

  if (partition == "default" && !off_the_record)
    partition = "";

  BrowserContextMap& registry = off_the_record
                                    ? off_the_record_browser_context_map()
                                    : browser_context_map();
  AppRuntimeBrowserContext* browser_context = registry[partition].get();
  if (!browser_context) {
    browser_context = new AppRuntimeBrowserContext(partition, off_the_record,
                                                   allow_to_load_extensions);
    registry[partition] =
        std::unique_ptr<AppRuntimeBrowserContext>(browser_context);
  }
  return browser_context;
}

// static
void AppRuntimeBrowserContext::Clear() {
  auto default_context = std::move(browser_context_map()[""]);
  off_the_record_browser_context_map().clear();
  browser_context_map().clear();
}

AppRuntimeBrowserContext::~AppRuntimeBrowserContext() {
  if (off_the_record_) {
    ForEachLoadedStoragePartition(
        [](content::StoragePartition* storage_partition) {
          BrowsingDataRemover* remover =
              BrowsingDataRemover::GetForStoragePartition(storage_partition);
          remover->Remove(
              BrowsingDataRemover::Unbounded(),
              BrowsingDataRemover::RemoveBrowsingDataMask::REMOVE_ALL);
        });
  }

  NotifyWillBeDestroyed();

  user_pref_service_->CommitPendingWrite();
#if defined(USE_NEVA_CHROME_EXTENSIONS)
  local_state_->CommitPendingWrite();
#endif

  BrowserContextDependencyManager::GetInstance()->DestroyBrowserContextServices(
      this);

  ShutdownStoragePartitions();
}

base::FilePath AppRuntimeBrowserContext::GetPath() {
  return path_;
}

bool AppRuntimeBrowserContext::IsOffTheRecord() {
  return off_the_record_;
}

content::DownloadManagerDelegate*
AppRuntimeBrowserContext::GetDownloadManagerDelegate() {
  if (!download_manager_delegate_) {
    download_manager_delegate_ =
        std::make_unique<AppRuntimeDownloadManagerDelegate>();
  }
  return download_manager_delegate_.get();
}

content::BrowserPluginGuestManager*
AppRuntimeBrowserContext::GetGuestManager() {
  return nullptr;
}

storage::SpecialStoragePolicy*
AppRuntimeBrowserContext::GetSpecialStoragePolicy() {
  return nullptr;
}

content::PlatformNotificationService*
AppRuntimeBrowserContext::GetPlatformNotificationService() {
  return nullptr;
}

content::PushMessagingService*
AppRuntimeBrowserContext::GetPushMessagingService() {
  return nullptr;
}

content::StorageNotificationService*
AppRuntimeBrowserContext::GetStorageNotificationService() {
  return nullptr;
}

content::SSLHostStateDelegate*
AppRuntimeBrowserContext::GetSSLHostStateDelegate() {
  return nullptr;
}

std::unique_ptr<content::ZoomLevelDelegate>
AppRuntimeBrowserContext::CreateZoomLevelDelegate(const base::FilePath&) {
  return nullptr;
}

content::PermissionControllerDelegate*
AppRuntimeBrowserContext::GetPermissionControllerDelegate() {
  return AppRuntimePermissionManagerFactory::GetForBrowserContext(this);
}

content::ReduceAcceptLanguageControllerDelegate*
AppRuntimeBrowserContext::GetReduceAcceptLanguageControllerDelegate() {
  return nullptr;
}

content::BackgroundFetchDelegate*
AppRuntimeBrowserContext::GetBackgroundFetchDelegate() {
  return nullptr;
}

content::BackgroundSyncController*
AppRuntimeBrowserContext::GetBackgroundSyncController() {
  return nullptr;
}

content::BrowsingDataRemoverDelegate*
AppRuntimeBrowserContext::GetBrowsingDataRemoverDelegate() {
  return nullptr;
}

bool AppRuntimeBrowserContext::ExtensionsIsAllowed() {
  return extensions_is_allowed_;
}

void AppRuntimeBrowserContext::FlushCookieStore() {
  GetDefaultStoragePartition()
      ->GetCookieManagerForBrowserProcess()
      ->FlushCookieStore(
          network::mojom::CookieManager::FlushCookieStoreCallback());
}

std::string AppRuntimeBrowserContext::GetName() const {
  return name_;
}

content::ClientHintsControllerDelegate*
AppRuntimeBrowserContext::GetClientHintsControllerDelegate() {
  return new neva_user_agent::ClientHints();
}

AppRuntimeBrowserContext::AppRuntimeBrowserContext(
    const std::string& partition,
    bool off_the_record,
    bool allow_to_load_extensions)
    : off_the_record_(off_the_record),
      extensions_is_allowed_(allow_to_load_extensions),
      name_(partition),
      path_(InitPath(partition)) {
  user_pref_service_ = prefs::CreateUserPrefService(this);
#if defined(USE_NEVA_CHROME_EXTENSIONS)
  local_state_ = prefs::CreateLocalState(GetPath());
  sessions::SessionIdGenerator::GetInstance()->Init(local_state_.get());

  neva::NevaExtensionsBrowserClient* extensions_browser_client =
      static_cast<neva::NevaExtensionsBrowserClient*>(
          extensions::ExtensionsBrowserClient::Get());
  extensions_browser_client->AssociateWithBrowserContext(
      this, user_pref_service_.get());

  // Create BrowserContextKeyedServices now that we have an
  // ExtensionsBrowserClient that BrowserContextKeyedAPIServices can query.
  BrowserContextDependencyManager::GetInstance()->CreateBrowserContextServices(
      this);

  raw_ptr<neva::NevaExtensionSystem> extension_system =
      static_cast<neva::NevaExtensionSystem*>(
          extensions::ExtensionSystem::Get(this));
  extension_system->InitForRegularProfile(extensions_is_allowed_);
  extension_system->FinishInitialization();

  if (extensions_is_allowed_) {
    neva::LoadExtensionsFromCommandLine(extension_system);
  }

  neva::NevaExtensionsServiceImpl* extension_service =
      neva::NevaExtensionsServiceFactory::GetService(this);
  if (extension_service) {
    extension_service->SetTabHelper(std::make_unique<neva::TabHelperImpl>());
  }
#endif  // defined(USE_NEVA_CHROME_EXTENSIONS)
}

// static
AppRuntimeBrowserContext::BrowserContextMap&
AppRuntimeBrowserContext::browser_context_map() {
  static base::NoDestructor<AppRuntimeBrowserContext::BrowserContextMap>
      browser_context_registry;
  return *browser_context_registry;
}

// static
AppRuntimeBrowserContext::BrowserContextMap&
AppRuntimeBrowserContext::off_the_record_browser_context_map() {
  static base::NoDestructor<AppRuntimeBrowserContext::BrowserContextMap>
      off_the_record_registry;
  return *off_the_record_registry;
}

base::FilePath AppRuntimeBrowserContext::InitPath(
    const std::string& partition) const {
  // Default value
  base::FilePath path;
  base::PathService::Get(base::DIR_CACHE, &path);

  // Overwrite default path value
  base::CommandLine* cmd_line = base::CommandLine::ForCurrentProcess();
  if (cmd_line->HasSwitch(kUserDataDir)) {
    base::FilePath new_path = cmd_line->GetSwitchValuePath(kUserDataDir);
    if (!new_path.empty()) {
      path = new_path;
      LOG(INFO) << "kUserDataDir is set.";
    } else {
      LOG(INFO) << "kUserDataDir is empty.";
    }
  } else {
    LOG(INFO) << "kUserDataDir isn't set.";
  }

  // Append storage name
  if (!off_the_record_) {
    if (partition.empty()) {
      path = path.Append(FILE_PATH_LITERAL("Default"));
    } else {
      path = path.Append(FILE_PATH_LITERAL("Partitions"))
                 .Append(base::FilePath::FromUTF8Unsafe(
                     base::EscapePath(base::ToLowerASCII(partition))));
    }
  }

  LOG(INFO) << "Will use path=" << path.value();
  return path;
}

}  // namespace neva_app_runtime
