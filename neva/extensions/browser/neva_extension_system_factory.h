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

#ifndef NEVA_EXTENSIONS_BROWSER_NEVA_EXTENSION_SYSTEM_FACTORY_H_
#define NEVA_EXTENSIONS_BROWSER_NEVA_EXTENSION_SYSTEM_FACTORY_H_

#include "base/memory/singleton.h"
#include "extensions/browser/extension_system_provider.h"

namespace neva {

class NevaExtensionSystemFactory : public extensions::ExtensionSystemProvider {
 public:
  NevaExtensionSystemFactory(const NevaExtensionSystemFactory&) = delete;
  NevaExtensionSystemFactory& operator=(const NevaExtensionSystemFactory&) =
      delete;

  // ExtensionSystemProvider implementation:
  extensions::ExtensionSystem* GetForBrowserContext(
      content::BrowserContext* context) override;

  static NevaExtensionSystemFactory* GetInstance();

 private:
  friend struct base::DefaultSingletonTraits<NevaExtensionSystemFactory>;

  NevaExtensionSystemFactory();
  ~NevaExtensionSystemFactory() override;

  // BrowserContextKeyedServiceFactory implementation:
  KeyedService* BuildServiceInstanceFor(
      content::BrowserContext* context) const override;
  content::BrowserContext* GetBrowserContextToUse(
      content::BrowserContext* context) const override;
  bool ServiceIsCreatedWithBrowserContext() const override;
};

}  // namespace neva

#endif  // NEVA_EXTENSIONS_BROWSER_NEVA_EXTENSION_SYSTEM_FACTORY_H_
