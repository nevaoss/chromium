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

#ifndef NEVA_BROWSER_SHELL_SERVICE_BROWSER_SHELL_COOKIE_MANAGER_IMPL_H_
#define NEVA_BROWSER_SHELL_SERVICE_BROWSER_SHELL_COOKIE_MANAGER_IMPL_H_

#include "base/memory/raw_ptr.h"
#include "mojo/public/cpp/bindings/associated_remote.h"
#include "mojo/public/cpp/bindings/pending_associated_receiver.h"
#include "neva/browser_shell/service/public/mojom/browser_shell_cookie_manager.mojom.h"
#include "services/network/public/mojom/cookie_manager.mojom.h"

namespace browser_shell {

class CookieManagerImpl : public mojom::CookieManager {
 public:
  CookieManagerImpl(const std::string& partition);
  CookieManagerImpl(const CookieManagerImpl&) = delete;
  CookieManagerImpl& operator=(const CookieManagerImpl&) = delete;
  ~CookieManagerImpl() override;

  // mojom::CookieManager
  void BindClient(BindClientCallback callback) override;
  void SetCookieOption(int32_t option,
                       SetCookieOptionCallback callback) override;
  void ClearAllCookies(ClearAllCookiesCallback callback) override;
  void GetAllCookiesForTesting(
      GetAllCookiesForTestingCallback callback) override;

  bool IsCookieEnabled() { return first_party_cookie_; }

 private:
  mojo::AssociatedRemote<mojom::CookieManagerClient> remote_client_;

  enum class CookieOption {
    kAllowedAll = 1,
    kBlockedAll,
    kBlockedThirdParty,
  };

  void OnGetAllCookies(const net::CookieList& cookies);
  void BlockThirdPartyCookies(bool is_blocked);

  bool first_party_cookie_ = true;
  bool third_party_cookie_blocked_ = false;

  GetAllCookiesForTestingCallback get_all_cookies_callback_;

  raw_ptr<network::mojom::CookieManager> network_cookie_manager_ = nullptr;
  CookieOption cookie_option_ = CookieOption::kAllowedAll;
};

}  // namespace browser_shell

#endif  // NEVA_BROWSER_SHELL_SERVICE_BROWSER_SHELL_COOKIE_MANAGER_IMPL_H_
