// Copyright 2019 LG Electronics, Inc.
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

#ifndef NEVA_APP_RUNTIME_BROWSER_APP_RUNTIME_WEBVIEW_CONTROLLER_IMPL_H_
#define NEVA_APP_RUNTIME_BROWSER_APP_RUNTIME_WEBVIEW_CONTROLLER_IMPL_H_

#include "base/memory/raw_ptr.h"
#include "content/public/browser/render_frame_host_receiver_set.h"
#include "content/public/browser/web_contents_user_data.h"
#include "neva/app_runtime/public/mojom/app_runtime_webview_controller.mojom.h"

namespace content {

class WebContents;

}  // namespace content

namespace neva_app_runtime {

class WebViewControllerDelegate;

class AppRuntimeWebViewControllerImpl
    : public content::WebContentsUserData<AppRuntimeWebViewControllerImpl>,
      public mojom::AppRuntimeWebViewController {
 public:
  static void BindAppRuntimeWebViewController(
      mojo::PendingAssociatedReceiver<mojom::AppRuntimeWebViewController>
          receiver,
      content::RenderFrameHost* rfh);

  AppRuntimeWebViewControllerImpl(const AppRuntimeWebViewControllerImpl&) =
      delete;
  AppRuntimeWebViewControllerImpl& operator=(
      const AppRuntimeWebViewControllerImpl&) = delete;

  ~AppRuntimeWebViewControllerImpl() override;

  void SetDelegate(WebViewControllerDelegate* delegate);

  // using CallFunctionCallback = base::OnceCallback<void(const std::string&)>;
  void CallFunction(const std::string& name,
                    const std::vector<std::string>& args,
                    CallFunctionCallback callback) override;

  void SendCommand(const std::string& name,
                   const std::vector<std::string>& args) override;
 private:
  friend class content::WebContentsUserData<AppRuntimeWebViewControllerImpl>;
  explicit AppRuntimeWebViewControllerImpl(content::WebContents* web_contents);

  content::RenderFrameHostReceiverSet<mojom::AppRuntimeWebViewController>
      receivers_;
  raw_ptr<WebViewControllerDelegate> webview_controller_delegate_ = nullptr;

  WEB_CONTENTS_USER_DATA_KEY_DECL();
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_BROWSER_APP_RUNTIME_WEBVIEW_CONTROLLER_IMPL_H_
