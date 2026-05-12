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

#include "neva/extensions/browser/browser_context_keyed_service_factories.h"

#include "neva/extensions/browser/api/cookies/cookies_api.h"
#include "neva/extensions/browser/api/extension_action/extension_action_api.h"
#include "neva/extensions/browser/api/web_navigation/web_navigation_api.h"
#include "neva/extensions/browser/neva_extensions_service_factory.h"

namespace neva {

void EnsureBrowserContextKeyedServiceFactoriesBuilt() {
  CookiesAPI::GetFactoryInstance();
  ExtensionActionAPI::GetFactoryInstance();
  NevaExtensionsServiceFactory::GetFactoryInstance();
  WebNavigationAPI::GetFactoryInstance();
}

}  // namespace neva
