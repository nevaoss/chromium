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

#ifndef NEVA_APP_RUNTIME_APP_APP_RUNTIME_SITE_SETTINGS_H_
#define NEVA_APP_RUNTIME_APP_APP_RUNTIME_SITE_SETTINGS_H_

#include <string>
#include <string_view>
#include <vector>

#include "base/memory/raw_ptr.h"

namespace neva_app_runtime {

class AppRuntimeBrowserContext;

class AppRuntimeSiteSettings {
 public:
  struct Permission {
    std::string type;
    std::string setting;
  };

  AppRuntimeSiteSettings(AppRuntimeBrowserContext* context);
  AppRuntimeSiteSettings(std::string_view partition_spec);
  ~AppRuntimeSiteSettings();

  std::vector<std::string> GetAllSites() const;
  std::vector<std::string> GetSitesForSettingType(std::string_view type) const;
  std::vector<Permission> GetOriginPermissions(
      std::string_view origin,
      const std::vector<std::string>& types) const;
  void ResetOriginPermissions(std::string_view origin,
                              const std::vector<std::string>& types);
 private:
  raw_ptr<AppRuntimeBrowserContext> context_;
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_APP_APP_RUNTIME_SITE_SETTINGS_H_
