// Copyright 2025 LG Electronics, Inc.
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
// chrome/browser/media/webrtc/media_stream_device_permission_context.cc
//
// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "neva/app_runtime/browser/media/webrtc/media_stream_device_permission_context.h"

#include "components/content_settings/core/browser/host_content_settings_map.h"
#include "components/content_settings/core/common/content_settings_constraints.h"
#include "components/content_settings/core/common/content_settings_pattern.h"
#include "components/content_settings/core/common/content_settings_types.h"
#include "components/permissions/permission_request_manager.h"
#include "components/permissions/permission_util.h"
#include "components/permissions/permissions_client.h"
#include "content/public/browser/browser_thread.h"
#include "services/network/public/mojom/permissions_policy/permissions_policy_feature.mojom.h"
#include "url/gurl.h"

namespace {

network::mojom::PermissionsPolicyFeature GetPermissionsPolicyFeature(
    ContentSettingsType type) {
  if (type == ContentSettingsType::MEDIASTREAM_MIC) {
    return network::mojom::PermissionsPolicyFeature::kMicrophone;
  }

  DCHECK_EQ(ContentSettingsType::MEDIASTREAM_CAMERA, type);
  return network::mojom::PermissionsPolicyFeature::kCamera;
}

}  // namespace

namespace neva_app_runtime {

MediaStreamDevicePermissionContext::MediaStreamDevicePermissionContext(
    content::BrowserContext* browser_context,
    ContentSettingsType content_settings_type)
    : ContentSettingPermissionContextBase(
          browser_context,
          content_settings_type,
          GetPermissionsPolicyFeature(content_settings_type)),
      content_settings_type_(content_settings_type) {
  DCHECK(content_settings_type_ == ContentSettingsType::MEDIASTREAM_MIC ||
         content_settings_type_ == ContentSettingsType::MEDIASTREAM_CAMERA);
}

MediaStreamDevicePermissionContext::~MediaStreamDevicePermissionContext() {}

void MediaStreamDevicePermissionContext::DecidePermission(
    std::unique_ptr<permissions::PermissionRequestData> request_data,
    permissions::BrowserPermissionCallback callback) {
  // PermissionRequestManager::CreateForWebContents must have been called
  // during web contents initialization.
  permissions::ContentSettingPermissionContextBase::DecidePermission(
      std::move(request_data), std::move(callback));
}

ContentSetting
MediaStreamDevicePermissionContext::GetContentSettingStatusInternal(
    content::RenderFrameHost* render_frame_host,
    const GURL& requesting_origin,
    const GURL& embedding_origin) const {
  ContentSetting content_setting = permissions::
      ContentSettingPermissionContextBase::GetContentSettingStatusInternal(
          render_frame_host, requesting_origin, embedding_origin);

  if (content_setting == CONTENT_SETTING_DEFAULT) {
    content_setting = CONTENT_SETTING_ASK;
  }

  return content_setting;
}

void MediaStreamDevicePermissionContext::ResetPermission(
    const GURL& requesting_origin,
    const GURL& embedding_origin) {
  NOTREACHED() << "ResetPermission is not implemented";
}

bool MediaStreamDevicePermissionContext::IsRestrictedToSecureOrigins() const {
  return true;
}

}  // namespace neva_app_runtime
