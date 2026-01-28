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
// Based on chrome/browser/permissions/permission_manager_factory.cc
//
// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "neva/app_runtime/browser/permissions/app_runtime_permission_manager_factory.h"

#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "components/permissions/contexts/camera_pan_tilt_zoom_permission_context.h"
#include "components/permissions/permission_util.h"
#include "components/permissions/request_type.h"
#include "components/webrtc/media_stream_device_enumerator_impl.h"
#include "content/public/browser/browser_context.h"
#include "neva/app_runtime/browser/media/webrtc/media_stream_device_permission_context.h"
#include "neva/app_runtime/browser/permissions/app_runtime_automatic_fullscreen_permission_context.h"
#include "neva/app_runtime/browser/permissions/app_runtime_denied_permission_context.h"
#include "services/network/public/mojom/permissions_policy/permissions_policy_feature.mojom-shared.h"
#include "third_party/blink/public/common/permissions/permission_utils.h"

namespace neva_app_runtime {
namespace {

class CameraPanTiltZoomPermissionBypassDelegate
    : public permissions::CameraPanTiltZoomPermissionContext::Delegate {
 public:
  CameraPanTiltZoomPermissionBypassDelegate() = default;
  ~CameraPanTiltZoomPermissionBypassDelegate() override = default;

  bool GetPermissionStatusInternal(
      const GURL& requesting_origin,
      const GURL& embedding_origin,
      ContentSetting* content_setting_result) override {
    return false;
  }
};

webrtc::MediaStreamDeviceEnumerator* GetMediaStreamDeviceEnumerator() {
  static base::NoDestructor<webrtc::MediaStreamDeviceEnumeratorImpl> instance;
  return instance.get();
}

permissions::PermissionManager::PermissionContextMap CreatePermissionContexts(
    content::BrowserContext* context) {
  // Create default permission contexts initially.
  permissions::PermissionManager::PermissionContextMap permission_contexts;

  permission_contexts[ContentSettingsType::AUTOMATIC_FULLSCREEN] =
      std::make_unique<
          neva_app_runtime::AppRuntimeAutomaticFullscreenPermissionContext>(
              context);

  permission_contexts[ContentSettingsType::MEDIASTREAM_MIC] =
      std::make_unique<neva_app_runtime::MediaStreamDevicePermissionContext>(
          context, ContentSettingsType::MEDIASTREAM_MIC);

  permission_contexts[ContentSettingsType::MEDIASTREAM_CAMERA] =
      std::make_unique<neva_app_runtime::MediaStreamDevicePermissionContext>(
          context, ContentSettingsType::MEDIASTREAM_CAMERA);

  permission_contexts[ContentSettingsType::CAMERA_PAN_TILT_ZOOM] =
      std::make_unique<permissions::CameraPanTiltZoomPermissionContext>(
          context,
          std::make_unique<CameraPanTiltZoomPermissionBypassDelegate>(),
          GetMediaStreamDeviceEnumerator());

  // For now, all requests are denied. As features are added, their permission
  // contexts can be added here instead of DeniedPermissionContext.
  for (blink::PermissionType type : blink::GetAllPermissionTypes()) {
    ContentSettingsType content_settings_type =
        permissions::PermissionUtil::PermissionTypeToContentSettingsType(type);
    if (permissions::PermissionUtil::IsPermission(content_settings_type) &&
        permission_contexts.find(content_settings_type) ==
            permission_contexts.end()) {
      permission_contexts[content_settings_type] =
          std::make_unique<AppRuntimeDeniedPermissionContext>(
              context, content_settings_type,
              network::mojom::PermissionsPolicyFeature::kNotFound);
    }
  }
  return permission_contexts;
}
}  // namespace

// static
permissions::PermissionManager*
AppRuntimePermissionManagerFactory::GetForBrowserContext(
    content::BrowserContext* context) {
  return static_cast<permissions::PermissionManager*>(
      GetInstance()->GetServiceForBrowserContext(context, true));
}

// static
AppRuntimePermissionManagerFactory*
AppRuntimePermissionManagerFactory::GetInstance() {
  static base::NoDestructor<AppRuntimePermissionManagerFactory> instance;
  return instance.get();
}

AppRuntimePermissionManagerFactory::AppRuntimePermissionManagerFactory()
    : BrowserContextKeyedServiceFactory(
          "AppRuntimePermissionManagerFactory",
          BrowserContextDependencyManager::GetInstance()) {}

AppRuntimePermissionManagerFactory::~AppRuntimePermissionManagerFactory() {}

// BrowserContextKeyedServiceFactory methods:
std::unique_ptr<KeyedService>
AppRuntimePermissionManagerFactory::BuildServiceInstanceForBrowserContext(
    content::BrowserContext* context) const {
  return std::make_unique<permissions::PermissionManager>(
      context, CreatePermissionContexts(context));
}

}  // namespace neva_app_runtime
