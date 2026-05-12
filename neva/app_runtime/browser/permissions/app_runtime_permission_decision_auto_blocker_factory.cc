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
// weblayer/browser/permissions/permission_decision_auto_blocker_factory.cc
//
// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "neva/app_runtime/browser/permissions/app_runtime_permission_decision_auto_blocker_factory.h"

#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "components/permissions/permission_decision_auto_blocker.h"
#include "neva/app_runtime/browser/host_content_settings_map_factory.h"

namespace neva_app_runtime {

// static
permissions::PermissionDecisionAutoBlocker*
AppRuntimePermissionDecisionAutoBlockerFactory::GetForBrowserContext(
    content::BrowserContext* browser_context) {
  return static_cast<permissions::PermissionDecisionAutoBlocker*>(
      GetInstance()->GetServiceForBrowserContext(browser_context, true));
}

// static
AppRuntimePermissionDecisionAutoBlockerFactory*
AppRuntimePermissionDecisionAutoBlockerFactory::GetInstance() {
  static base::NoDestructor<AppRuntimePermissionDecisionAutoBlockerFactory>
      factory;
  return factory.get();
}

AppRuntimePermissionDecisionAutoBlockerFactory::
    AppRuntimePermissionDecisionAutoBlockerFactory()
    : BrowserContextKeyedServiceFactory(
          "PermissionDecisionAutoBlocker",
          BrowserContextDependencyManager::GetInstance()) {
  DependsOn(HostContentSettingsMapFactory::GetInstance());
}

AppRuntimePermissionDecisionAutoBlockerFactory::
    ~AppRuntimePermissionDecisionAutoBlockerFactory() = default;

KeyedService*
AppRuntimePermissionDecisionAutoBlockerFactory::BuildServiceInstanceFor(
    content::BrowserContext* context) const {
  return new permissions::PermissionDecisionAutoBlocker(
      HostContentSettingsMapFactory::GetForBrowserContext(context));
}

content::BrowserContext*
AppRuntimePermissionDecisionAutoBlockerFactory::GetBrowserContextToUse(
    content::BrowserContext* context) const {
  return context;
}

}  // namespace neva_app_runtime
