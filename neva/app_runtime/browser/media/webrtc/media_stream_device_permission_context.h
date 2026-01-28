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
// chrome/browser/media/webrtc/media_stream_device_permission_context.h
//
// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NEVA_APP_RUNTIME_BROWSER_MEDIA_WEBRTC_MEDIA_STREAM_DEVICE_PERMISSION_CONTEXT_H_
#define NEVA_APP_RUNTIME_BROWSER_MEDIA_WEBRTC_MEDIA_STREAM_DEVICE_PERMISSION_CONTEXT_H_

#include "components/content_settings/core/common/content_settings_types.h"
#include "components/permissions/content_setting_permission_context_base.h"

class GURL;

namespace permissions {
class PermissionRequestID;
}

namespace content {
class BrowserContext;
class RenderFrameHost;
class WebContents;
}  // namespace content

namespace neva_app_runtime {

using BrowserPermissionCallback = base::OnceCallback<void(ContentSetting)>;

class MediaStreamDevicePermissionContext
    : public permissions::ContentSettingPermissionContextBase {
 public:
  explicit MediaStreamDevicePermissionContext(
      content::BrowserContext* browser_context,
      ContentSettingsType content_settings_type);
  ~MediaStreamDevicePermissionContext() override;

  // ContentSettingPermissionContextBase:
  void DecidePermission(
      std::unique_ptr<permissions::PermissionRequestData> request_data,
      permissions::BrowserPermissionCallback callback) override;

  ContentSetting GetContentSettingStatusInternal(
      content::RenderFrameHost* render_frame_host,
      const GURL& requesting_origin,
      const GURL& embedding_origin) const override;

  void ResetPermission(const GURL& requesting_origin,
                       const GURL& embedding_origin) override;

 private:
  bool IsRestrictedToSecureOrigins() const override;

  ContentSettingsType content_settings_type_;
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_BROWSER_MEDIA_WEBRTC_MEDIA_STREAM_DEVICE_PERMISSION_CONTEXT_H_
