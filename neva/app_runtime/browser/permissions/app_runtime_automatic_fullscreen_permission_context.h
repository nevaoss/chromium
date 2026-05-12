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

#ifndef NEVA_APP_RUNTIME_BROWSER_PERMISSIONS_APP_RUNTIME_AUTOMATIC_FULLSCREEN_PERMISSION_CONTEXT_H__
#define NEVA_APP_RUNTIME_BROWSER_PERMISSIONS_APP_RUNTIME_AUTOMATIC_FULLSCREEN_PERMISSION_CONTEXT_H__

#include "components/permissions/contexts/automatic_fullscreen_permission_context.h"

namespace neva_app_runtime {

class AppRuntimeAutomaticFullscreenPermissionContext
    : public permissions::AutomaticFullscreenPermissionContext {
 public:
  explicit AppRuntimeAutomaticFullscreenPermissionContext(
      content::BrowserContext* browser_context);
  ~AppRuntimeAutomaticFullscreenPermissionContext() override = default;

  AppRuntimeAutomaticFullscreenPermissionContext(
      const AppRuntimeAutomaticFullscreenPermissionContext&) = delete;
  AppRuntimeAutomaticFullscreenPermissionContext& operator=(
      const AppRuntimeAutomaticFullscreenPermissionContext&) = delete;

 protected:
  ContentSetting GetPermissionStatusInternal(
      content::RenderFrameHost* render_frame_host,
      const GURL& requesting_origin,
      const GURL& embedding_origin) const override;
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_BROWSER_PERMISSIONS_APP_RUNTIME_AUTOMATIC_FULLSCREEN_PERMISSION_CONTEXT_H__
