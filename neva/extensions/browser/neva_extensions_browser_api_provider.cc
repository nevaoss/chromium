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

#include "neva/extensions/browser/neva_extensions_browser_api_provider.h"

#include "extensions/browser/extension_function_registry.h"
#include "neva/extensions/browser/api/generated_api_registration.h"

namespace neva {

NevaExtensionsBrowserAPIProvider::NevaExtensionsBrowserAPIProvider() = default;
NevaExtensionsBrowserAPIProvider::~NevaExtensionsBrowserAPIProvider() = default;

void NevaExtensionsBrowserAPIProvider::RegisterExtensionFunctions(
    ExtensionFunctionRegistry* registry) {
  // Preferences.
  // registry->RegisterFunction<GetPreferenceFunction>();
  // registry->RegisterFunction<SetPreferenceFunction>();
  // registry->RegisterFunction<ClearPreferenceFunction>();

  // Generated APIs from Chrome.
  NevaGeneratedFunctionRegistry::RegisterAll(registry);
}

}  // namespace neva
