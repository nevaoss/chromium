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
// Based on chrome/browser/permissions/permission_manager_factory.h
//
// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NEVA_APP_RUNTIME_BROWSER_PERMISSIONS_APP_RUNTIME_PERMISSION_MANAGER_FACTORY_H_
#define NEVA_APP_RUNTIME_BROWSER_PERMISSIONS_APP_RUNTIME_PERMISSION_MANAGER_FACTORY_H_

#include "base/no_destructor.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"
#include "components/permissions/permission_manager.h"

namespace content {
class BrowserContext;
}

namespace permissions {
class PermissionManager;
}

namespace neva_app_runtime {

class AppRuntimePermissionManagerFactory
    : public BrowserContextKeyedServiceFactory {
 public:
  static permissions::PermissionManager* GetForBrowserContext(
      content::BrowserContext* context);
  static AppRuntimePermissionManagerFactory* GetInstance();

 private:
  friend class base::NoDestructor<AppRuntimePermissionManagerFactory>;

  AppRuntimePermissionManagerFactory();
  AppRuntimePermissionManagerFactory(
      const AppRuntimePermissionManagerFactory&) = delete;
  AppRuntimePermissionManagerFactory& operator=(
      const AppRuntimePermissionManagerFactory&) = delete;
  ~AppRuntimePermissionManagerFactory() override;

  // BrowserContextKeyedServiceFactory methods:
  KeyedService* BuildServiceInstanceFor(
      content::BrowserContext* context) const override;
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_BROWSER_PERMISSIONS_APP_RUNTIME_PERMISSION_MANAGER_FACTORY_H_
