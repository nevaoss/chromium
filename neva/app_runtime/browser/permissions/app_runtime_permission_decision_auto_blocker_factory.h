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
// weblayer/browser/permissions/permission_decision_auto_blocker_factory.h
//
// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NEVA_APP_RUNTIME_BROWSER_PERMISSIONS_APP_RUNTIME_PERMISSION_DECISION_AUTO_BLOCKER_FACTORY_H_
#define NEVA_APP_RUNTIME_BROWSER_PERMISSIONS_APP_RUNTIME_PERMISSION_DECISION_AUTO_BLOCKER_FACTORY_H_

#include "base/no_destructor.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"

namespace permissions {
class PermissionDecisionAutoBlocker;
}

namespace neva_app_runtime {

class AppRuntimePermissionDecisionAutoBlockerFactory
    : public BrowserContextKeyedServiceFactory {
 public:
  AppRuntimePermissionDecisionAutoBlockerFactory(
      const AppRuntimePermissionDecisionAutoBlockerFactory&) = delete;
  AppRuntimePermissionDecisionAutoBlockerFactory& operator=(
      const AppRuntimePermissionDecisionAutoBlockerFactory&) = delete;

  static permissions::PermissionDecisionAutoBlocker* GetForBrowserContext(
      content::BrowserContext* browser_context);
  static AppRuntimePermissionDecisionAutoBlockerFactory* GetInstance();

 private:
  friend class base::NoDestructor<
      AppRuntimePermissionDecisionAutoBlockerFactory>;

  AppRuntimePermissionDecisionAutoBlockerFactory();
  ~AppRuntimePermissionDecisionAutoBlockerFactory() override;

  // BrowserContextKeyedServiceFactory
  KeyedService* BuildServiceInstanceFor(
      content::BrowserContext* context) const override;

  content::BrowserContext* GetBrowserContextToUse(
      content::BrowserContext* context) const override;
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_BROWSER_PERMISSIONS_APP_RUNTIME_PERMISSION_DECISION_AUTO_BLOCKER_FACTORY_H_
