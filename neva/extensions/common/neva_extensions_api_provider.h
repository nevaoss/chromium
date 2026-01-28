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

#ifndef NEVA_EXTENSIONS_COMMON_NEVA_EXTENSIONS_API_PROVIDER_H_
#define NEVA_EXTENSIONS_COMMON_NEVA_EXTENSIONS_API_PROVIDER_H_

#include "extensions/common/extensions_api_provider.h"

namespace neva {

class NevaExtensionsAPIProvider : public extensions::ExtensionsAPIProvider {
 public:
  NevaExtensionsAPIProvider();
  NevaExtensionsAPIProvider(const NevaExtensionsAPIProvider&) = delete;
  NevaExtensionsAPIProvider& operator=(const NevaExtensionsAPIProvider&) =
      delete;
  ~NevaExtensionsAPIProvider() override;

  // ExtensionsAPIProvider:
  void AddAPIFeatures(extensions::FeatureProvider* provider) override;
  void AddManifestFeatures(extensions::FeatureProvider* provider) override;
  void AddPermissionFeatures(extensions::FeatureProvider* provider) override;
  void AddBehaviorFeatures(extensions::FeatureProvider* provider) override;
  void AddAPIJSONSources(
      extensions::JSONFeatureProviderSource* json_source) override;
  bool IsAPISchemaGenerated(const std::string& name) override;
  std::string_view GetAPISchema(const std::string& name) override;
  void RegisterPermissions(
      extensions::PermissionsInfo* permissions_info) override;
  void RegisterManifestHandlers() override;
};

}  // namespace neva

#endif  // NEVA_EXTENSIONS_COMMON_NEVA_EXTENSIONS_API_PROVIDER_H_
