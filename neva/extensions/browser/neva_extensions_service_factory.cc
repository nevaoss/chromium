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

#include "neva/extensions/browser/neva_extensions_service_factory.h"

#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "neva/app_runtime/browser/app_runtime_browser_context.h"
#include "neva/extensions/browser/neva_extensions_service_impl.h"
#include "neva/extensions/browser/neva_extensions_services_manager_impl.h"

namespace neva {

NevaExtensionsServiceFactory::NevaExtensionsServiceFactory()
    : BrowserContextKeyedServiceFactory(
          "NevaExtensionsService",
          BrowserContextDependencyManager::GetInstance()) {}

NevaExtensionsServiceFactory::~NevaExtensionsServiceFactory() = default;

// static
NevaExtensionsServiceImpl* NevaExtensionsServiceFactory::GetService(
    content::BrowserContext* context) {
  return static_cast<NevaExtensionsServiceImpl*>(
      GetFactoryInstance()->GetServiceForBrowserContext(context, true));
}

// static
NevaExtensionsServiceFactory*
NevaExtensionsServiceFactory::GetFactoryInstance() {
  static base::NoDestructor<NevaExtensionsServiceFactory> factory;
  return factory.get();
}

KeyedService* NevaExtensionsServiceFactory::BuildServiceInstanceFor(
    content::BrowserContext* browser_context) const {
  return NevaExtensionsServicesManagerImpl::GetInstance()
      ->GetNevaExtensionsServiceFor(browser_context);
}

content::BrowserContext* NevaExtensionsServiceFactory::GetBrowserContextToUse(
    content::BrowserContext* context) const {
  if (static_cast<neva_app_runtime::AppRuntimeBrowserContext*>(context)
          ->ExtensionsIsAllowed()) {
    return context;
  }

  return nullptr;
}

}  // namespace neva
