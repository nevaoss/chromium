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
// Based on chrome/browser/permissions/chrome_permissions_client.cc
//
// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "neva/app_runtime/browser/permissions/app_runtime_permissions_client.h"

#include "components/content_settings/core/browser/cookie_settings.h"
#include "neva/app_runtime/browser/host_content_settings_map_factory.h"
#include "neva/app_runtime/browser/permissions/app_runtime_origin_keyed_permission_action_service_factory.h"
#include "neva/app_runtime/browser/permissions/app_runtime_permission_decision_auto_blocker_factory.h"
#include "neva/app_runtime/browser/permissions/app_runtime_permission_manager_factory.h"
#include "neva/app_runtime/browser/permissions/app_runtime_permission_prompt.h"
#include "neva/app_runtime/browser/permissions/app_runtime_permission_prompt_factory.h"

namespace neva_app_runtime {

AppRuntimePermissionsClient::~AppRuntimePermissionsClient() = default;

// static
AppRuntimePermissionsClient* AppRuntimePermissionsClient::GetInstance() {
  static base::NoDestructor<AppRuntimePermissionsClient> instance;
  return instance.get();
}

void AppRuntimePermissionsClient::SetPromptFactory(
    base::WeakPtr<AppRuntimePermissionPromptFactory> prompt_factory) {
  prompt_factory_ = std::move(prompt_factory);
}

HostContentSettingsMap* AppRuntimePermissionsClient::GetSettingsMap(
    content::BrowserContext* browser_context) {
  return neva_app_runtime::HostContentSettingsMapFactory::GetForBrowserContext(
      browser_context);
}

scoped_refptr<content_settings::CookieSettings>
AppRuntimePermissionsClient::GetCookieSettings(
    content::BrowserContext* browser_context) {
  return scoped_refptr<content_settings::CookieSettings>(nullptr);
}

privacy_sandbox::TrackingProtectionSettings*
AppRuntimePermissionsClient::GetTrackingProtectionSettings(
    content::BrowserContext* browser_context) {
  return nullptr;
}

bool AppRuntimePermissionsClient::IsSubresourceFilterActivated(
    content::BrowserContext* browser_context,
    const GURL& url) {
  return false;
}

permissions::OriginKeyedPermissionActionService*
AppRuntimePermissionsClient::GetOriginKeyedPermissionActionService(
    content::BrowserContext* browser_context) {
  return AppRuntimeOriginKeyedPermissionActionServiceFactory::
      GetForBrowserContext(browser_context);
}

permissions::PermissionActionsHistory*
AppRuntimePermissionsClient::GetPermissionActionsHistory(
    content::BrowserContext* browser_context) {
  NOTIMPLEMENTED();
  return nullptr;
}

permissions::PermissionDecisionAutoBlocker*
AppRuntimePermissionsClient::GetPermissionDecisionAutoBlocker(
    content::BrowserContext* browser_context) {
  return AppRuntimePermissionDecisionAutoBlockerFactory::GetForBrowserContext(
      browser_context);
}

permissions::ObjectPermissionContextBase*
AppRuntimePermissionsClient::GetChooserContext(
    content::BrowserContext* browser_context,
    ContentSettingsType type) {
  NOTIMPLEMENTED();
  return nullptr;
}

bool AppRuntimePermissionsClient::CanBypassEmbeddingOriginCheck(
    const GURL& requesting_origin,
    const GURL& embedding_origin) {
  NOTIMPLEMENTED();
  return true;
}

std::unique_ptr<permissions::PermissionPrompt>
AppRuntimePermissionsClient::CreatePrompt(
    content::WebContents* web_contents,
    permissions::PermissionPrompt::Delegate* delegate) {
  if (prompt_factory_) {
    return prompt_factory_->CreatePermissionPrompt(web_contents, delegate);
  }
  return std::unique_ptr<permissions::PermissionPrompt>();
}

AppRuntimePermissionsClient::AppRuntimePermissionsClient() = default;

}  // namespace neva_app_runtime
