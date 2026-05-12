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

#include "neva/app_runtime/browser/permissions/app_runtime_automatic_fullscreen_permission_context.h"

#include "base/logging.h"
#include "content/public/browser/web_contents.h"
#include "neva/app_runtime/app/app_runtime_page_contents.h"

namespace neva_app_runtime {

AppRuntimeAutomaticFullscreenPermissionContext::
    AppRuntimeAutomaticFullscreenPermissionContext(
        content::BrowserContext* browser_context)
    : AutomaticFullscreenPermissionContext(browser_context) {}

ContentSetting
AppRuntimeAutomaticFullscreenPermissionContext::GetPermissionStatusInternal(
    content::RenderFrameHost* render_frame_host,
    const GURL& requesting_origin,
    const GURL& embedding_origin) const {
  if (render_frame_host) {
    content::WebContents* web_contents =
        content::WebContents::FromRenderFrameHost(render_frame_host);
    PageContents* page_contents = PageContents::From(web_contents);
    if (page_contents &&
        !page_contents->IsTransientActivationForHtmlFullscreenRequired()) {
      return CONTENT_SETTING_ALLOW;
    }
  }

  return AutomaticFullscreenPermissionContext::GetPermissionStatusInternal(
      render_frame_host, requesting_origin, embedding_origin);
}

}  // namespace neva_app_runtime
