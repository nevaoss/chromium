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

#ifndef NEVA_EXTENSIONS_COMMON_PERMISSIONS_NEVA_API_PERMISSIONS_H_
#define NEVA_EXTENSIONS_COMMON_PERMISSIONS_NEVA_API_PERMISSIONS_H_

#include "base/containers/span.h"
#include "extensions/common/alias.h"
#include "extensions/common/permissions/api_permission.h"

namespace neva {
namespace api_permissions {

// Returns the information necessary to construct chrome-layer extension
// APIPermissions.
base::span<const extensions::APIPermissionInfo::InitInfo> GetPermissionInfos();

// Returns the list of aliases for chrome-layer extension APIPermissions.
base::span<const extensions::Alias> GetPermissionAliases();

}  // namespace api_permissions
}  // namespace neva

#endif  // NEVA_EXTENSIONS_COMMON_PERMISSIONS_NEVA_API_PERMISSIONS_H_
