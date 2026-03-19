// Copyright 2023 LG Electronics, Inc.
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

#include "neva/extensions/browser/neva_extension_host_delegate.h"

#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_delegate.h"

namespace neva {

NevaExtensionHostDelegate::NevaExtensionHostDelegate() = default;

NevaExtensionHostDelegate::~NevaExtensionHostDelegate() = default;

void NevaExtensionHostDelegate::OnExtensionHostCreated(
    content::WebContents* web_contents) {}

void NevaExtensionHostDelegate::CreateTab(
    std::unique_ptr<content::WebContents> web_contents,
    const GURL& target_url,
    const std::string& extension_id,
    WindowOpenDisposition disposition,
    const blink::mojom::WindowFeatures& window_features,
    bool user_gesture) {}

void NevaExtensionHostDelegate::ProcessMediaAccessRequest(
    content::WebContents* web_contents,
    const content::MediaStreamRequest& request,
    content::MediaResponseCallback callback,
    const extensions::Extension* extension) {}

bool NevaExtensionHostDelegate::CheckMediaAccessPermission(
    content::RenderFrameHost* render_frame_host,
    const url::Origin& security_origin,
    blink::mojom::MediaStreamType type,
    const extensions::Extension* extension) {
  return true;
}

content::PictureInPictureResult
NevaExtensionHostDelegate::EnterPictureInPicture(
    content::WebContents* web_contents) {
  return content::PictureInPictureResult::kNotSupported;
}

void NevaExtensionHostDelegate::ExitPictureInPicture() {}

}  // namespace neva
