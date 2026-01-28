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

#include "neva/app_runtime/app/app_runtime_site_settings.h"

#include <algorithm>
#include <array>
#include <set>

#include "components/content_settings/core/browser/host_content_settings_map.h"
#include "components/content_settings/core/common/content_settings.h"
#include "components/content_settings/core/common/content_settings_types.h"
#include "components/content_settings/core/common/content_settings_utils.h"
#include "neva/app_runtime/app/app_runtime_shell_window.h"
#include "neva/app_runtime/browser/app_runtime_browser_context.h"
#include "neva/app_runtime/browser/app_runtime_storage_partition_spec.h"
#include "neva/app_runtime/browser/host_content_settings_map_factory.h"
#include "url/gurl.h"
#include "url/origin.h"

namespace neva_app_runtime {

struct ContentSettingsTypeNameEntry {
  ContentSettingsType type;
  const char* name;
};

// The list below provides ContentSettings that can be decided by the user
// via PermissionPrompt, for all others in
// app_runtime/browser/permissions/app_runtime_permission_manager_factory.cc
// file a deny permission context is created by default.
constexpr auto kAvailableContentSettingsTypesNames =
    std::to_array<const ContentSettingsTypeNameEntry>({
        {ContentSettingsType::CAMERA_PAN_TILT_ZOOM, "camera-pan-tilt-zoom"},
        {ContentSettingsType::MEDIASTREAM_MIC, "media-stream-mic"},
        {ContentSettingsType::MEDIASTREAM_CAMERA, "media-stream-camera"},
    });

bool IsInAvailableSiteSettingsTypes(ContentSettingsType type) {
  return std::find_if(kAvailableContentSettingsTypesNames.cbegin(),
                      kAvailableContentSettingsTypesNames.cend(),
                      [type](const auto& entry) {
                        return type == entry.type;
                      }) != kAvailableContentSettingsTypesNames.cend();
}

ContentSettingsType ContentSettingsTypeFromName(std::string_view name) {
  for (const auto& entry : kAvailableContentSettingsTypesNames) {
    if (entry.name == name) {
      return entry.type;
    }
  }
  return ContentSettingsType::DEFAULT;
}

std::vector<std::pair<ContentSettingsType, std::string>>
BuildContentSettingsTypesFrom(const std::vector<std::string>& types) {
  std::vector<std::pair<ContentSettingsType, std::string>> result;
  if (types.empty()) {
    for (const auto& entry : kAvailableContentSettingsTypesNames) {
      result.push_back(std::make_pair(entry.type, std::string(entry.name)));
    }
  } else {
    for (const auto& type : types) {
      ContentSettingsType content_type = ContentSettingsTypeFromName(type);
      if (content_type != ContentSettingsType::DEFAULT) {
        result.push_back(std::make_pair(content_type, type));
      }
    }
  }
  return result;
}

std::vector<ContentSettingPatternSource>
GetSingleOriginExceptionsForContentType(HostContentSettingsMap* settings_map,
                                        ContentSettingsType content_type) {
  ContentSettingsForOneType entries =
      settings_map->GetSettingsForOneType(content_type);

  std::erase_if(entries, [](const ContentSettingPatternSource& e) {
    return !content_settings::PatternAppliesToSingleOrigin(
               e.primary_pattern, e.secondary_pattern) ||
           e.source == content_settings::ProviderType::kWebuiAllowlistProvider;
  });
  return entries;
}

AppRuntimeSiteSettings::AppRuntimeSiteSettings(
    AppRuntimeBrowserContext* context)
    : context_(context) {}

AppRuntimeSiteSettings::AppRuntimeSiteSettings(std::string_view partition_spec)
    : AppRuntimeSiteSettings(AppRuntimeBrowserContext::From(
          ParseStoragePartitionSpec(partition_spec))) {}

AppRuntimeSiteSettings::~AppRuntimeSiteSettings() = default;

std::vector<std::string> AppRuntimeSiteSettings::GetAllSites() const {
  auto* settings_map =
      HostContentSettingsMapFactory::GetForBrowserContext(context_);

  std::set<std::string> origin_set;
  for (const auto& entry : kAvailableContentSettingsTypesNames) {
    auto exceptions =
        GetSingleOriginExceptionsForContentType(settings_map, entry.type);
    for (const auto& e : exceptions) {
      auto origin = url::Origin::Create(GURL(e.primary_pattern.ToString()));
      origin_set.insert(origin.Serialize());
    }
  }

  std::vector<std::string> origins;
  std::move(origin_set.begin(), origin_set.end(), std::back_inserter(origins));
  return origins;
}

std::vector<std::string> AppRuntimeSiteSettings::GetSitesForSettingType(
    std::string_view type) const {
  auto* settings_map =
      HostContentSettingsMapFactory::GetForBrowserContext(context_);
  if (!settings_map) {
    return {};
  }

  ContentSettingsType content_type = ContentSettingsTypeFromName(type);
  if (content_type == ContentSettingsType::DEFAULT) {
    return {};
  }

  auto exceptions =
      GetSingleOriginExceptionsForContentType(settings_map, content_type);
  std::set<std::string> origin_set;
  for (const auto& e : exceptions) {
    auto origin = url::Origin::Create(GURL(e.primary_pattern.ToString()));
    origin_set.insert(origin.Serialize());
  }

  std::vector<std::string> origins;
  std::move(origin_set.begin(), origin_set.end(), std::back_inserter(origins));
  return origins;
}

std::vector<AppRuntimeSiteSettings::Permission>
AppRuntimeSiteSettings::GetOriginPermissions(
    std::string_view origin,
    const std::vector<std::string>& types) const {
  GURL origin_url(origin);
  if (!origin_url.is_valid()) {
    return {};
  }

  auto* settings_map =
      HostContentSettingsMapFactory::GetForBrowserContext(context_);
  if (!settings_map) {
    return {};
  }

  std::vector<Permission> permissions;
  auto content_type_pairs = BuildContentSettingsTypesFrom(types);
  for (const auto& entry : content_type_pairs) {
    ContentSetting setting = settings_map->GetUserModifiableContentSetting(
        origin_url, origin_url, entry.first);

    if (setting != CONTENT_SETTING_DEFAULT) {
      Permission perm;
      perm.type = std::move(entry.second);
      perm.setting = content_settings::ContentSettingToString(setting);
      permissions.push_back(std::move(perm));
    }
  }

  return permissions;
}

void AppRuntimeSiteSettings::ResetOriginPermissions(
    std::string_view origin,
    const std::vector<std::string>& types) {
  GURL origin_url(origin);
  if (!origin_url.is_valid()) {
    return;
  }

  auto* settings_map =
      HostContentSettingsMapFactory::GetForBrowserContext(context_);
  if (!settings_map) {
    return;
  }

  auto content_type_pairs = BuildContentSettingsTypesFrom(types);
  for (const auto& entry : content_type_pairs) {
    ContentSetting setting = settings_map->GetUserModifiableContentSetting(
        origin_url, origin_url, entry.first);
    if (setting != CONTENT_SETTING_DEFAULT) {
      settings_map->SetContentSettingDefaultScope(
          origin_url, origin_url, entry.first, CONTENT_SETTING_DEFAULT);
    }
  }
}

}  // namespace neva_app_runtime
