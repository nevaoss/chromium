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
// chrome/browser/permissions/origin_keyed_permission_action_service_factory.cc
//
// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "neva/app_runtime/browser/permissions/app_runtime_origin_keyed_permission_action_service_factory.h"

#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "components/permissions/origin_keyed_permission_action_service.h"

namespace neva_app_runtime {

// static
permissions::OriginKeyedPermissionActionService*
AppRuntimeOriginKeyedPermissionActionServiceFactory::GetForBrowserContext(
    content::BrowserContext* browser_context) {
  return static_cast<permissions::OriginKeyedPermissionActionService*>(
      GetInstance()->GetServiceForBrowserContext(browser_context, true));
}

// static
AppRuntimeOriginKeyedPermissionActionServiceFactory*
AppRuntimeOriginKeyedPermissionActionServiceFactory::GetInstance() {
  static base::NoDestructor<AppRuntimeOriginKeyedPermissionActionServiceFactory>
      instance;
  return instance.get();
}

AppRuntimeOriginKeyedPermissionActionServiceFactory::
    AppRuntimeOriginKeyedPermissionActionServiceFactory()
    : BrowserContextKeyedServiceFactory(
          "OriginKeyedPermissionActionService",
          BrowserContextDependencyManager::GetInstance()) {}

AppRuntimeOriginKeyedPermissionActionServiceFactory::
    ~AppRuntimeOriginKeyedPermissionActionServiceFactory() = default;

KeyedService*
AppRuntimeOriginKeyedPermissionActionServiceFactory::BuildServiceInstanceFor(
    content::BrowserContext* context) const {
  return new permissions::OriginKeyedPermissionActionService();
}

}  // namespace neva_app_runtime
