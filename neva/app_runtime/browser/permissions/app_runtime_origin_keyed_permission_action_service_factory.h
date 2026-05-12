// Copyright 2024 LG Electronics, Inc.
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
//
// Based on
// chrome/browser/permissions/origin_keyed_permission_action_service_factory.h
//
// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NEVA_APP_RUNTIME_BROWSER_PERMISSIONS_APP_RUNTIME_ORIGIN_KEYED_PERMISSION_ACTION_SERVICE_FACTORY_H_
#define NEVA_APP_RUNTIME_BROWSER_PERMISSIONS_APP_RUNTIME_ORIGIN_KEYED_PERMISSION_ACTION_SERVICE_FACTORY_H_

#include "base/no_destructor.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"

namespace permissions {
class OriginKeyedPermissionActionService;
}

namespace neva_app_runtime {

// Factory to create a service to keep track of permission actions of the
// current browser session for metrics evaluation.
class AppRuntimeOriginKeyedPermissionActionServiceFactory
    : public BrowserContextKeyedServiceFactory {
 public:
  AppRuntimeOriginKeyedPermissionActionServiceFactory(
      const AppRuntimeOriginKeyedPermissionActionServiceFactory&) = delete;
  AppRuntimeOriginKeyedPermissionActionServiceFactory& operator=(
      const AppRuntimeOriginKeyedPermissionActionServiceFactory&) = delete;

  static permissions::OriginKeyedPermissionActionService* GetForBrowserContext(
      content::BrowserContext* browser_context);
  static AppRuntimeOriginKeyedPermissionActionServiceFactory* GetInstance();

 private:
  friend class base::NoDestructor<
      AppRuntimeOriginKeyedPermissionActionServiceFactory>;

  AppRuntimeOriginKeyedPermissionActionServiceFactory();

  ~AppRuntimeOriginKeyedPermissionActionServiceFactory() override;

  // BrowserContextKeyedServiceFactory:
  KeyedService* BuildServiceInstanceFor(
      content::BrowserContext* context) const override;
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_BROWSER_PERMISSIONS_APP_RUNTIME_ORIGIN_KEYED_PERMISSION_ACTION_SERVICE_FACTORY_H_
