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

#include "neva/extensions/browser/neva_extension_system_factory.h"

#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "extensions/browser/extension_prefs_factory.h"
#include "extensions/browser/extension_registry_factory.h"
#include "neva/extensions/browser/neva_extension_system.h"

using content::BrowserContext;

namespace neva {

extensions::ExtensionSystem* NevaExtensionSystemFactory::GetForBrowserContext(
    BrowserContext* context) {
  return static_cast<NevaExtensionSystem*>(
      GetInstance()->GetServiceForBrowserContext(context, true));
}

// static
NevaExtensionSystemFactory* NevaExtensionSystemFactory::GetInstance() {
  return base::Singleton<NevaExtensionSystemFactory>::get();
}

NevaExtensionSystemFactory::NevaExtensionSystemFactory()
    : ExtensionSystemProvider("NevaExtensionSystem",
                              BrowserContextDependencyManager::GetInstance()) {
  DependsOn(extensions::ExtensionPrefsFactory::GetInstance());
  DependsOn(extensions::ExtensionRegistryFactory::GetInstance());
}

NevaExtensionSystemFactory::~NevaExtensionSystemFactory() {}

KeyedService* NevaExtensionSystemFactory::BuildServiceInstanceFor(
    BrowserContext* context) const {
  return new NevaExtensionSystem(context);
}

BrowserContext* NevaExtensionSystemFactory::GetBrowserContextToUse(
    BrowserContext* context) const {
  // Use a separate instance for incognito.
  return context;
}

bool NevaExtensionSystemFactory::ServiceIsCreatedWithBrowserContext() const {
  return true;
}

}  // namespace neva
