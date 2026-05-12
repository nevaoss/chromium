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

#include "neva/browser_shell/service/browser_shell_service_impl.h"

#include <map>
#include <memory>
#include <optional>
#include <set>

#include "base/functional/bind.h"
#include "base/json/json_reader.h"
#include "base/logging.h"
#include "base/values.h"
#include "neva/app_runtime/app/app_runtime_page_contents.h"
#include "neva/app_runtime/app/app_runtime_page_view.h"
#include "neva/app_runtime/app/app_runtime_shell.h"
#include "neva/app_runtime/app/app_runtime_shell_environment.h"
#include "neva/app_runtime/app/app_runtime_shell_window.h"
#include "neva/app_runtime/browser/app_runtime_browser_context.h"
#include "neva/app_runtime/browser/app_runtime_storage_partition_name.h"
#include "neva/browser_shell/service/browser_shell_cookie_manager_impl.h"
#include "neva/browser_shell/service/browser_shell_ipc_endpoint_impl.h"
#include "neva/browser_shell/service/browser_shell_main_window_impl.h"
#include "neva/browser_shell/service/browser_shell_page_contents_impl.h"
#include "neva/browser_shell/service/browser_shell_page_view_impl.h"
#include "neva/browser_shell/service/browser_shell_permissions_impl.h"
#include "neva/browser_shell/service/browser_shell_webrequest_impl.h"
#include "neva/injection/public/common/webapi_names.h"
#include "neva/logging.h"

namespace browser_shell {

namespace {

std::map<std::string, std::string> GetAPIsFromPageContentParams(
    const base::Value::Dict& dict) {
  std::map<std::string, std::string> result;
  const base::Value::List* api = dict.FindList("api");
  if (!api)
    return result;

  for (const auto& item : *api) {
    if (item.is_string())
      result.emplace(item.GetString(), "{}");
  }
  return result;
}

std::string GetStringValueFromDict(
    const base::Value::Dict& dict,
    const std::string& key,
    const std::string& default_value = std::string()) {
  const std::string* value = dict.FindString(key);
  return value ? *value : default_value;
}

neva_app_runtime::PageContents::Type GetTypeFromDict(
    const base::Value::Dict& dict) {
  std::string type_str = GetStringValueFromDict(dict, "page-contents-type");
  auto type = neva_app_runtime::PageContents::ConvertStringToType(type_str);

  if (type == neva_app_runtime::PageContents::Type::kMain) {
    LOG(ERROR) << "Applcation cannot create PageContents with type \"main\"";
  }

  return type;
}

void SetContentParams(neva_app_runtime::PageContents::CreateParams& params,
                      const base::Value::Dict& dict) {
  params.injections = GetAPIsFromPageContentParams(dict);
  params.user_agent = GetStringValueFromDict(dict, "user-agent");
  neva_app_runtime::StoragePartitionInfo partition_info =
      neva_app_runtime::ParseStoragePartitionName(
          GetStringValueFromDict(dict, "partition"));
  params.storage_partition_name = partition_info.name;
  params.storage_partition_off_the_record = partition_info.off_the_record;
  params.default_access_to_media = dict.FindBool("media-access");
  params.allow_file_access_from_file_urls =
      dict.FindBool("allow-file-access").value_or(false);
  params.allow_universal_access_from_file_urls =
      dict.FindBool("allow-universal-access").value_or(false);
  params.require_transient_activation_for_html_fullscreen =
      dict.FindBool("require-transient-activation-for-html-fullscreen")
          .value_or(true);
  params.error_page_hiding = dict.FindBool("error-page-hiding").value_or(false);
  params.zoom_factor = dict.FindDouble("zoom-factor");
  params.type = GetTypeFromDict(dict);
  params.site_page_contents_id =
      dict.FindIntByDottedPath("site-page-contents.id");
}

void CheckTabID(neva_app_runtime::PageContents* page_contents) {
  std::optional<uint64_t> opt =
      neva_app_runtime::ShellEnvironment::GetInstance()->GetTabID(
          page_contents->GetWebContents());

  if (opt.has_value())
    LOG(INFO) << "TabID for Tab is " << opt.value();
  else
    LOG(INFO) << "There was no tab specified";
}

}  // namespace

ShellServiceImpl::ShellServiceImpl(
    std::unique_ptr<neva_app_runtime::Shell> shell)
    : shell_(std::move(shell)),
      permissions_impl_(std::make_unique<PermissionsImpl>(this)) {
  shell_->AddObserver(this);
}

ShellServiceImpl::~ShellServiceImpl() = default;

void ShellServiceImpl::AddBinding(
    mojo::PendingReceiver<mojom::ShellService> receiver) {
  receivers_.Add(this, std::move(receiver));
}

void ShellServiceImpl::RegisterClient(
    mojo::PendingRemote<mojom::ShellServiceClient> remote) {
  auto id = remotes_.Add(std::move(remote));
  remotes_.Get(id)->SetLaunchParams(shell_->GetLaunchParams());
}

void ShellServiceImpl::BindShellWindow(
    mojo::PendingReceiver<mojom::ShellWindow> receiver,
    BindShellWindowCallback callback) {
  // It's only 1 shell window allowed yet.
  if (!main_shell_window_impl_) {
    auto* window = shell_->GetMainWindow();
    if (!window) {
      std::move(callback).Run(std::string());
      return;
    }
    main_shell_window_impl_ =
        std::make_unique<ShellMainWindowImpl>(this, window);
  }

  main_shell_window_impl_->AddBinding(std::move(receiver));
  std::move(callback).Run(main_shell_window_impl_->GetName());
}

void ShellServiceImpl::BindPermissions(
    mojo::PendingReceiver<mojom::Permissions> receiver) {
  if (!permissions_impl_) {
    permissions_impl_ = std::make_unique<PermissionsImpl>(this);
  }
  permissions_impl_->AddBinding(std::move(receiver));
}

void ShellServiceImpl::CreateCookieManager(
    mojo::PendingReceiver<mojom::CookieManager> receiver,
    const std::string& partition) {
  auto cookie_manager_impl = std::make_unique<CookieManagerImpl>(partition);
  cookie_manager_receivers_.Add(std::move(cookie_manager_impl),
                                std::move(receiver));
}

void ShellServiceImpl::CreatePageView(
    mojo::PendingReceiver<mojom::PageView> receiver,
    const std::string& json,
    CreatePageViewCallback callback) {
  auto page_view = std::make_unique<neva_app_runtime::PageView>();
  auto content_params = shell_->GetDefaultContentsParams();
  std::optional<base::Value> val = base::JSONReader::Read(json);
  if (val && val->is_dict()) {
    base::Value::Dict* dict = val->GetDict().FindDict("page-contents-params");
    if (dict)
      SetContentParams(content_params, *dict);
  }

  auto page_contents =
      std::make_unique<neva_app_runtime::PageContents>(content_params);

#if NEVA_DCHECK_IS_ON()
  neva_app_runtime::PageContents* page_contents_ptr = page_contents.get();
#endif

  page_view->SetPageContents(std::move(page_contents));
  auto page_view_impl = std::make_unique<PageViewImpl>(this, page_view.get());

  neva_app_runtime::ShellEnvironment::GetInstance()->SaveDetachedView(
      std::move(page_view));

  const uint64_t id = page_view_impl->GetID();
  AddUniqueReceiver(std::move(page_view_impl), std::move(receiver));
  std::move(callback).Run(id);

#if NEVA_DCHECK_IS_ON()
  CheckTabID(page_contents_ptr);
#endif
}

void ShellServiceImpl::CreatePageContents(
    mojo::PendingReceiver<mojom::PageContents> receiver,
    const std::string& json,
    CreatePageContentsCallback callback) {
  auto content_params = shell_->GetDefaultContentsParams();
  std::optional<base::Value> val = base::JSONReader::Read(json);
  if (val && val->is_dict())
    SetContentParams(content_params, val->GetDict());

  auto page_contents =
      std::make_unique<neva_app_runtime::PageContents>(content_params);

#if NEVA_DCHECK_IS_ON()
  neva_app_runtime::PageContents* page_contents_ptr = page_contents.get();
#endif

  auto page_contents_impl =
      std::make_unique<PageContentsImpl>(this, page_contents.get());

  neva_app_runtime::ShellEnvironment::GetInstance()->SaveDetachedContents(
      std::move(page_contents));

  const uint64_t id = page_contents_impl->GetID();
  auto info = browser_shell::mojom::PageContentsCreationInfo::New(
      page_contents_impl->GetActiveState(),
      page_contents_impl->GetErrorPageHiding(),
      page_contents_impl->GetUserAgent(),
      page_contents_impl->GetZoomFactor());

  AddUniqueReceiver(std::move(page_contents_impl), std::move(receiver));
  std::move(callback).Run(id, std::move(info));

#if NEVA_DCHECK_IS_ON()
  CheckTabID(page_contents_ptr);
#endif
}

void ShellServiceImpl::CreateShellIpcEndpoint(
    mojo::PendingReceiver<mojom::ShellIpcEndpoint> receiver,
    const std::string& channel) {
  auto shell_ipc_endpoint_impl = std::make_unique<ShellIpcEndpointImpl>(
      channel, &shell_ipc_);
  ipc_endpoint_receivers_.Add(
      std::move(shell_ipc_endpoint_impl), std::move(receiver));
}

void ShellServiceImpl::TouchSession(const std::string& partition) {
  neva_app_runtime::StoragePartitionInfo partition_info =
      neva_app_runtime::ParseStoragePartitionName(partition);

  std::ignore =
    neva_app_runtime::AppRuntimeBrowserContext::From(
          partition_info.name, partition_info.off_the_record);
}

void ShellServiceImpl::CreateWebRequest(
    mojo::PendingReceiver<mojom::WebRequest> receiver,
    const std::string& partition) {
  auto webrequest_impl = std::make_unique<WebRequestImpl>(partition);
  webrequest_receivers_.Add(std::move(webrequest_impl), std::move(receiver));
}

void ShellServiceImpl::AddUniqueReceiver(
    std::unique_ptr<PageContentsImpl> page_contents_impl,
    mojo::PendingReceiver<mojom::PageContents> receiver) {
  page_contents_receivers_.Add(
      std::move(page_contents_impl), std::move(receiver));
}

void ShellServiceImpl::AddUniqueReceiver(
    std::unique_ptr<PageViewImpl> page_view_impl,
    mojo::PendingReceiver<mojom::PageView> receiver) {
  page_view_receivers_.Add(std::move(page_view_impl), std::move(receiver));
}

void ShellServiceImpl::OnMainWindowClosing() {
  page_view_receivers_.Clear();
  page_contents_receivers_.Clear();
  ipc_endpoint_receivers_.Clear();
}

}  // namespace browser_shell
