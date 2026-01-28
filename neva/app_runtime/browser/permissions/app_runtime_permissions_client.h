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
// Based on chrome/browser/permissions/chrome_permissions_client.h
//
// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NEVA_APP_RUNTIME_BROWSER_PERMISSIONS_APP_RUNTIME_PERMISSIONS_CLIENT_H_
#define NEVA_APP_RUNTIME_BROWSER_PERMISSIONS_APP_RUNTIME_PERMISSIONS_CLIENT_H_

#include "base/memory/weak_ptr.h"
#include "base/no_destructor.h"
#include "components/permissions/permissions_client.h"

namespace neva_app_runtime {

class AppRuntimePermissionPromptFactory;

class AppRuntimePermissionsClient : public permissions::PermissionsClient {
 public:
  AppRuntimePermissionsClient(const AppRuntimePermissionsClient&) = delete;
  AppRuntimePermissionsClient& operator=(const AppRuntimePermissionsClient&) =
      delete;

  ~AppRuntimePermissionsClient() override;

  static AppRuntimePermissionsClient* GetInstance();
  void SetPromptFactory(
      base::WeakPtr<AppRuntimePermissionPromptFactory> prompt_factory);

  // PermissionsClient:
  HostContentSettingsMap* GetSettingsMap(
      content::BrowserContext* browser_context) override;
  scoped_refptr<content_settings::CookieSettings> GetCookieSettings(
      content::BrowserContext* browser_context) override;
  privacy_sandbox::TrackingProtectionSettings* GetTrackingProtectionSettings(
      content::BrowserContext* browser_context) override;
  bool IsSubresourceFilterActivated(content::BrowserContext* browser_context,
                                    const GURL& url) override;
  permissions::OriginKeyedPermissionActionService*
  GetOriginKeyedPermissionActionService(
      content::BrowserContext* browser_context) override;
  permissions::PermissionActionsHistory* GetPermissionActionsHistory(
      content::BrowserContext* browser_context) override;
  permissions::PermissionDecisionAutoBlocker* GetPermissionDecisionAutoBlocker(
      content::BrowserContext* browser_context) override;
  permissions::ObjectPermissionContextBase* GetChooserContext(
      content::BrowserContext* browser_context,
      ContentSettingsType type) override;

  bool CanBypassEmbeddingOriginCheck(const GURL& requesting_origin,
                                     const GURL& embedding_origin) override;
  std::unique_ptr<permissions::PermissionPrompt> CreatePrompt(
      content::WebContents* web_contents,
      permissions::PermissionPrompt::Delegate* delegate) override;

 private:
  friend class base::NoDestructor<AppRuntimePermissionsClient>;
  base::WeakPtr<AppRuntimePermissionPromptFactory> prompt_factory_;

  AppRuntimePermissionsClient();
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_BROWSER_PERMISSIONS_APP_RUNTIME_PERMISSIONS_CLIENT_H_
