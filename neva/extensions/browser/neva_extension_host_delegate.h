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

#ifndef NEVA_EXTENSIONS_BROWSER_NEVA_EXTENSION_HOST_DELEGATE_H_
#define NEVA_EXTENSIONS_BROWSER_NEVA_EXTENSION_HOST_DELEGATE_H_

#include "extensions/browser/extension_host_delegate.h"

namespace neva {

class NevaExtensionHostDelegate : public extensions::ExtensionHostDelegate {
 public:
  NevaExtensionHostDelegate();
  ~NevaExtensionHostDelegate() override;

  void OnExtensionHostCreated(content::WebContents* web_contents) override;

  void OnMainFrameCreatedForBackgroundPage(
      extensions::ExtensionHost* host) override;

  content::JavaScriptDialogManager* GetJavaScriptDialogManager() override;

  void CreateTab(std::unique_ptr<content::WebContents> web_contents,
                 const std::string& extension_id,
                 WindowOpenDisposition disposition,
                 const blink::mojom::WindowFeatures& window_features,
                 bool user_gesture) override;

  void ProcessMediaAccessRequest(
      content::WebContents* web_contents,
      const content::MediaStreamRequest& request,
      content::MediaResponseCallback callback,
      const extensions::Extension* extension) override;

  bool CheckMediaAccessPermission(
      content::RenderFrameHost* render_frame_host,
      const url::Origin& security_origin,
      blink::mojom::MediaStreamType type,
      const extensions::Extension* extension) override;

  content::PictureInPictureResult EnterPictureInPicture(
      content::WebContents* web_contents) override;

  void ExitPictureInPicture() override;
};

}  // namespace neva

#endif  // NEVA_EXTENSIONS_BROWSER_NEVA_EXTENSION_HOST_DELEGATE_H_
