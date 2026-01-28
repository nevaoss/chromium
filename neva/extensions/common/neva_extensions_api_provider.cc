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

#include "neva/extensions/common/neva_extensions_api_provider.h"

#include "extensions/common/features/json_feature_provider_source.h"
#include "extensions/common/permissions/extensions_api_permissions.h"
#include "extensions/common/permissions/permissions_info.h"
#include "neva/extensions/common/api/api_features.h"
#include "neva/extensions/common/api/generated_schemas.h"
#include "neva/extensions/common/api/manifest_features.h"
#include "neva/extensions/common/api/permission_features.h"
#include "neva/extensions/common/permissions/neva_api_permissions.h"

namespace neva {

NevaExtensionsAPIProvider::NevaExtensionsAPIProvider() {}
NevaExtensionsAPIProvider::~NevaExtensionsAPIProvider() = default;

void NevaExtensionsAPIProvider::AddAPIFeatures(
    extensions::FeatureProvider* provider) {
  AddNevaAPIFeatures(provider);
}

void NevaExtensionsAPIProvider::AddManifestFeatures(
    extensions::FeatureProvider* provider) {
  AddNevaManifestFeatures(provider);
}

void NevaExtensionsAPIProvider::AddPermissionFeatures(
    extensions::FeatureProvider* provider) {
  AddNevaPermissionFeatures(provider);
}

void NevaExtensionsAPIProvider::AddBehaviorFeatures(
    extensions::FeatureProvider* provider) {}

void NevaExtensionsAPIProvider::AddAPIJSONSources(
    extensions::JSONFeatureProviderSource* json_source) {}

bool NevaExtensionsAPIProvider::IsAPISchemaGenerated(const std::string& name) {
  return extensions::api::NevaGeneratedSchemas::IsGenerated(name);
}

std::string_view NevaExtensionsAPIProvider::GetAPISchema(
    const std::string& name) {
  return extensions::api::NevaGeneratedSchemas::Get(name);
}

void NevaExtensionsAPIProvider::RegisterPermissions(
    extensions::PermissionsInfo* permissions_info) {
  permissions_info->RegisterPermissions(
      neva::api_permissions::GetPermissionInfos(),
      neva::api_permissions::GetPermissionAliases());
}

void NevaExtensionsAPIProvider::RegisterManifestHandlers() {}

}  // namespace neva
