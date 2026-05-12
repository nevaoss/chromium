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

#include "neva/browser_shell/service/browser_shell_cookie_manager_impl.h"

#include "content/public/browser/storage_partition.h"
#include "neva/app_runtime/browser/app_runtime_browser_context.h"
#include "neva/app_runtime/browser/app_runtime_storage_partition_name.h"
#include "neva/logging.h"
#include "services/network/cookie_manager.h"

namespace browser_shell {

CookieManagerImpl::CookieManagerImpl(const std::string& partition) {
  neva_app_runtime::StoragePartitionInfo partition_info =
      neva_app_runtime::ParseStoragePartitionName(std::move(partition));
  network_cookie_manager_ =
      neva_app_runtime::AppRuntimeBrowserContext::From(
          partition_info.name, partition_info.off_the_record)
          ->GetDefaultStoragePartition()
          ->GetCookieManagerForBrowserProcess();
}

CookieManagerImpl::~CookieManagerImpl() = default;

void CookieManagerImpl::BindClient(BindClientCallback callback) {
  std::move(callback).Run(remote_client_.BindNewEndpointAndPassReceiver());
}

void CookieManagerImpl::SetCookieOption(int32_t option,
                                        SetCookieOptionCallback callback) {
  // If same cookie_type is set already, return false
  if (static_cast<int>(cookie_option_) == option) {
    LOG(INFO) << "This Cookie option is already set!";
    std::move(callback).Run(false);
    return;
  }
  if (!network_cookie_manager_) {
    LOG(WARNING) << __func__ << "Invalid Cookie Manager Instance";
    std::move(callback).Run(false);
    return;
  }
  switch (static_cast<CookieOption>(option)) {
    case CookieOption::kAllowedAll:
      first_party_cookie_ = true;
      BlockThirdPartyCookies(false);
      break;
    case CookieOption::kBlockedAll:
      first_party_cookie_ = false;
      BlockThirdPartyCookies(true);
      break;
    case CookieOption::kBlockedThirdParty:
      first_party_cookie_ = true;
      BlockThirdPartyCookies(true);
      break;
    default:
      LOG(WARNING) << __func__ << "Invalid Cookie Option!";
      std::move(callback).Run(false);
      return;
  }
  cookie_option_ = static_cast<CookieOption>(option);
  std::move(callback).Run(true);
}

void CookieManagerImpl::BlockThirdPartyCookies(bool is_blocked) {
  if (third_party_cookie_blocked_ == is_blocked) {
    return;
  }

  third_party_cookie_blocked_ = is_blocked;
  network_cookie_manager_->BlockThirdPartyCookies(is_blocked);
}

void CookieManagerImpl::ClearAllCookies(ClearAllCookiesCallback callback) {
  if (!network_cookie_manager_) {
    LOG(WARNING) << __func__ << "Invalid Cookie Manager Instance";
    std::move(callback).Run(false);
    return;
  }

  auto filter = network::mojom::CookieDeletionFilter::New();
  network_cookie_manager_->DeleteCookies(
      std::move(filter),
      network::mojom::CookieManager::DeleteCookiesCallback());
  std::move(callback).Run(true);
}

void CookieManagerImpl::GetAllCookiesForTesting(
    GetAllCookiesForTestingCallback callback) {
  std::vector<std::string> cookie_list;
  if (!network_cookie_manager_) {
    std::move(callback).Run(cookie_list);
    return;
  }

  get_all_cookies_callback_ = std::move(callback);
  network_cookie_manager_->GetAllCookies(base::BindOnce(
      &CookieManagerImpl::OnGetAllCookies, base::Unretained(this)));
}

void CookieManagerImpl::OnGetAllCookies(const net::CookieList& cookies) {
  std::vector<std::string> cookie_list;
  for (const net::CanonicalCookie& cookie : cookies) {
    std::string cookie_entry_str = "[" + cookie.Domain() + ": " +
                                   cookie.Name() + " / " + cookie.Value() +
                                   "], ";
    cookie_list.push_back(cookie_entry_str);
  }

  // Get and add all cookies to cookie_list;
  std::move(get_all_cookies_callback_).Run(cookie_list);
}

}  // namespace browser_shell
