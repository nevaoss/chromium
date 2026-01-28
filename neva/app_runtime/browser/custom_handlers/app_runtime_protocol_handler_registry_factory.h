// Copyright 2023 LG Electronics, Inc.
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

// Based on
// //components/custom_handlers/simple_protocol_handler_registry_factory.h

// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NEVA_APP_RUNTIME_BROWSER_CUSTOM_HANDLERS_APP_RUNTIME_PROTOCOL_HANDLER_REGISTRY_FACTORY_H_
#define NEVA_APP_RUNTIME_BROWSER_CUSTOM_HANDLERS_APP_RUNTIME_PROTOCOL_HANDLER_REGISTRY_FACTORY_H_

#include "base/no_destructor.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"

namespace base {
template <typename T>
class NoDestructor;
}

namespace custom_handlers {
class ProtocolHandlerRegistry;
}

namespace neva_app_runtime {
// Simgleton that owns all the ProtocolHandlerRegistrys and associates them with
// BrowserContext instances.

class AppRuntimeProtocolHandlerRegistryFactory
    : public BrowserContextKeyedServiceFactory {
 public:
  // Returns the singleton instance of the ProtocolHandlerRegistryFactory.
  static AppRuntimeProtocolHandlerRegistryFactory* GetInstance();

  // Returns the ProtocolHandlerRegistry that provides intent registration for
  // |context|. Ownership stays with this factory object.
  // Allows the caller to indicate that the KeyedService should not be created
  // if it's not registered. This is particularly useful for testings purposes,
  // since the TestBrowserContext doesn't implement the TwoPhaseShutdown of
  // KeyedService instances.
  static custom_handlers::ProtocolHandlerRegistry* GetForBrowserContext(
      content::BrowserContext* context);

  AppRuntimeProtocolHandlerRegistryFactory(
      const AppRuntimeProtocolHandlerRegistryFactory&) = delete;
  AppRuntimeProtocolHandlerRegistryFactory& operator=(
      const AppRuntimeProtocolHandlerRegistryFactory&) = delete;

 protected:
  // BrowserContextKeyedServiceFactory implementation.
  bool ServiceIsCreatedWithBrowserContext() const override;
  bool ServiceIsNULLWhileTesting() const override;

 private:
  friend base::NoDestructor<AppRuntimeProtocolHandlerRegistryFactory>;

  AppRuntimeProtocolHandlerRegistryFactory();
  ~AppRuntimeProtocolHandlerRegistryFactory() override = default;

  // BrowserContextKeyedServiceFactory implementation.
  std::unique_ptr<KeyedService> BuildServiceInstanceForBrowserContext(
      content::BrowserContext* profile) const override;
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_BROWSER_CUSTOM_HANDLERS_APP_RUNTIME_PROTOCOL_HANDLER_REGISTRY_FACTORY_H_
