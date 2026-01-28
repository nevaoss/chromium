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

#include "neva/browser_shell/service/browser_shell_site_settings_impl.h"

namespace browser_shell {

SiteSettingsImpl::SiteSettingsImpl(std::string_view spec)
    : site_settings_(spec) {}

SiteSettingsImpl::~SiteSettingsImpl() = default;

void SiteSettingsImpl::RegisterClient(
    mojo::PendingAssociatedRemote<mojom::SiteSettingsClient> remote) {
  remote_client_.Bind(std::move(remote));
}

void SiteSettingsImpl::GetAllSites(GetAllSitesCallback callback) {
  std::move(callback).Run(site_settings_.GetAllSites());
}

void SiteSettingsImpl::GetSitesForSettingType(
    const std::string& type,
    GetSitesForSettingTypeCallback callback) {
  std::move(callback).Run(site_settings_.GetSitesForSettingType(type));
}

void SiteSettingsImpl::GetOriginPermissions(
    const std::string& origin,
    const std::vector<std::string>& types,
    GetOriginPermissionsCallback callback) {
  auto permissions = site_settings_.GetOriginPermissions(origin, types);
  std::vector<mojom::PermissionPtr> result;
  for (const auto& perm : permissions) {
    result.push_back(mojom::Permission::New(perm.type, perm.setting));
  }
  std::move(callback).Run(std::move(result));
}

void SiteSettingsImpl::ResetOriginPermissions(
    const std::string& origin,
    const std::vector<std::string>& types) {
  site_settings_.ResetOriginPermissions(origin, types);
}

}  // namespace browser_shell
