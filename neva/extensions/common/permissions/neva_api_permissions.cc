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

#include "neva/extensions/common/permissions/neva_api_permissions.h"

#include <stddef.h>

#include <memory>

#include "base/memory/ptr_util.h"
#include "extensions/common/permissions/api_permission.h"
#include "extensions/common/permissions/api_permission_set.h"
#include "extensions/common/permissions/permissions_info.h"
#include "extensions/common/permissions/settings_override_permission.h"

using extensions::mojom::APIPermissionID;

namespace neva {
namespace api_permissions {

namespace {

template <typename T>
std::unique_ptr<extensions::APIPermission> CreateAPIPermission(
    const extensions::APIPermissionInfo* permission) {
  return std::make_unique<T>(permission);
}

// WARNING: If you are modifying a permission message in this list, be sure to
// add the corresponding permission message rule to
// ChromePermissionMessageProvider::GetPermissionMessages as well.
constexpr extensions::APIPermissionInfo::InitInfo permissions_to_register[] = {
    // Register permissions for all extension types.

    // Register extension permissions.
    {APIPermissionID::kCookie, "cookies",
     extensions::APIPermissionInfo::kFlagRequiresManagementUIWarning},
    {APIPermissionID::kNotifications, "notifications",
     extensions::APIPermissionInfo::kFlagRequiresManagementUIWarning},
    {APIPermissionID::kScripting, "scripting",
     extensions::APIPermissionInfo::kFlagRequiresManagementUIWarning},
    {APIPermissionID::kWebNavigation, "webNavigation",
     extensions::APIPermissionInfo::kFlagRequiresManagementUIWarning},

    // Register private permissions.

    // Full url access permissions.

    // Platform-app permissions.

    // Settings override permissions.
};

}  // namespace

base::span<const extensions::APIPermissionInfo::InitInfo> GetPermissionInfos() {
  return base::make_span(permissions_to_register);
}

base::span<const extensions::Alias> GetPermissionAliases() {
  // In alias constructor, first value is the alias name; second value is the
  // real name. See also alias.h.
  static constexpr extensions::Alias aliases[] = {
      extensions::Alias("windows", "tabs")};
  // static constexpr extensions::Alias aliases[] = {};

  return base::make_span(aliases);
}

}  // namespace api_permissions
}  // namespace neva
