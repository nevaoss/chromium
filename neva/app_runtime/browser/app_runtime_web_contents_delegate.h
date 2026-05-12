// Copyright 2022 LG Electronics, Inc.
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

#ifndef NEVA_APP_RUNTIME_BROWSER_APP_RUNTIME_WEB_CONTENTS_DELEGATE_H_
#define NEVA_APP_RUNTIME_BROWSER_APP_RUNTIME_WEB_CONTENTS_DELEGATE_H_

#include "content/public/browser/web_contents_delegate.h"
#include "neva/app_runtime/public/app_runtime_constants.h"

namespace pal {
class NotificationManagerDelegate;
}

namespace neva_app_runtime {

class AppRuntimeWebContentsDelegate : public content::WebContentsDelegate {
 public:
  AppRuntimeWebContentsDelegate();
  ~AppRuntimeWebContentsDelegate() override;

  void RunFileChooser(content::RenderFrameHost* render_frame_host,
                      scoped_refptr<content::FileSelectListener> listener,
                      const blink::mojom::FileChooserParams& params) override;

  virtual void SetSSLCertErrorPolicy(SSLCertErrorPolicy policy);
  virtual SSLCertErrorPolicy GetSSLCertErrorPolicy() const;

  void RegisterProtocolHandler(content::RenderFrameHost* requesting_frame,
                               const std::string& protocol,
                               const GURL& url,
                               bool user_gesture) override;

  void UnregisterProtocolHandler(content::RenderFrameHost* requesting_frame,
                                 const std::string& protocol,
                                 const GURL& url,
                                 bool user_gesture) override;

 private:
  std::unique_ptr<pal::NotificationManagerDelegate>
      notification_manager_delegate_;
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_BROWSER_APP_RUNTIME_WEB_CONTENTS_DELEGATE_H_
