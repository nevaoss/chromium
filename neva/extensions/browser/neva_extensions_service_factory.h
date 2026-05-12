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

#ifndef NEVA_EXTENSIONS_BROWSER_NEVA_EXTENSIONS_SERVICE_FACTORY_H_
#define NEVA_EXTENSIONS_BROWSER_NEVA_EXTENSIONS_SERVICE_FACTORY_H_

#include "base/no_destructor.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"

namespace neva {

class NevaExtensionsServiceImpl;

class NevaExtensionsServiceFactory : public BrowserContextKeyedServiceFactory {
 public:
  NevaExtensionsServiceFactory(const NevaExtensionsServiceFactory&) = delete;
  NevaExtensionsServiceFactory& operator=(const NevaExtensionsServiceFactory&) =
      delete;

  static NevaExtensionsServiceImpl* GetService(
      content::BrowserContext* context);

  static NevaExtensionsServiceFactory* GetFactoryInstance();

 private:
  friend class base::NoDestructor<NevaExtensionsServiceFactory>;

  NevaExtensionsServiceFactory();
  ~NevaExtensionsServiceFactory() override;

  // BrowserContextKeyedServiceFactory
  KeyedService* BuildServiceInstanceFor(
      content::BrowserContext* context) const override;
  content::BrowserContext* GetBrowserContextToUse(
      content::BrowserContext* context) const override;
};

}  // namespace neva

#endif  // NEVA_EXTENSIONS_BROWSER_NEVA_EXTENSIONS_SERVICE_FACTORY_H_
