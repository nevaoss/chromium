// Copyright 2025 LG Electronics, Inc.
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

#ifndef NEVA_BROWSER_SHELL_SERVICE_BROWSER_SHELL_SITE_SETTINGS_IMPL_H_
#define NEVA_BROWSER_SHELL_SERVICE_BROWSER_SHELL_SITE_SETTINGS_IMPL_H_

#include <string_view>

#include "mojo/public/cpp/bindings/associated_remote.h"
#include "mojo/public/cpp/bindings/pending_associated_remote.h"
#include "neva/app_runtime/app/app_runtime_site_settings.h"
#include "neva/browser_shell/service/public/mojom/browser_shell_site_settings.mojom.h"

namespace browser_shell {

class SiteSettingsImpl : public mojom::SiteSettings {
 public:
  SiteSettingsImpl(std::string_view spec);
  SiteSettingsImpl(const SiteSettingsImpl&) = delete;
  SiteSettingsImpl& operator=(const SiteSettingsImpl&) = delete;
  ~SiteSettingsImpl() override;

  // mojom::WebRequest
  void RegisterClient(
      mojo::PendingAssociatedRemote<mojom::SiteSettingsClient> remote) override;
  void GetAllSites(GetAllSitesCallback callback) override;
  void GetSitesForSettingType(const std::string& type,
                              GetSitesForSettingTypeCallback callback) override;
  void GetOriginPermissions(const std::string& origin,
                            const std::vector<std::string>& types,
                            GetOriginPermissionsCallback callback) override;
  void ResetOriginPermissions(const std::string& origin,
                              const std::vector<std::string>& types) override;
 private:
  neva_app_runtime::AppRuntimeSiteSettings site_settings_;
  mojo::AssociatedRemote<mojom::SiteSettingsClient> remote_client_;
};

}  // namespace browser_shell

#endif  // NEVA_BROWSER_SHELL_SERVICE_BROWSER_SHELL_SITE_SETTINGS_IMPL_H_
