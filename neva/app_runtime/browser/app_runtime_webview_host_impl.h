// Copyright 2016-2019 LG Electronics, Inc.
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

#ifndef NEVA_APP_RUNTIME_BROWSER_APP_RUNTIME_WEBVIEW_HOST_IMPL_H_
#define NEVA_APP_RUNTIME_BROWSER_APP_RUNTIME_WEBVIEW_HOST_IMPL_H_

#include "base/memory/raw_ptr.h"
#include "content/public/browser/render_frame_host_receiver_set.h"
#include "content/public/browser/web_contents_user_data.h"
#include "neva/app_runtime/public/mojom/app_runtime_webview.mojom.h"
#include "third_party/blink/public/mojom/neva/app_runtime_blink_delegate.mojom.h"

namespace content {

class WebContents;

}  // namespace content

namespace pal {
class NotificationManagerDelegate;
}

namespace neva_app_runtime {

class WebViewDelegate;

class AppRuntimeWebViewHostImpl
    : public content::WebContentsUserData<AppRuntimeWebViewHostImpl>,
      public mojom::AppRuntimeWebViewHost,
      public blink::mojom::AppRuntimeBlinkDelegate {
 public:
  AppRuntimeWebViewHostImpl(const AppRuntimeWebViewHostImpl&) = delete;
  AppRuntimeWebViewHostImpl& operator=(const AppRuntimeWebViewHostImpl&) =
      delete;
  ~AppRuntimeWebViewHostImpl() override;

  static void BindAppRuntimeWebViewHost(
      mojo::PendingAssociatedReceiver<mojom::AppRuntimeWebViewHost> receiver,
      content::RenderFrameHost* rfh);

  static void BindAppRuntimeBlinkDelegate(
      mojo::PendingAssociatedReceiver<blink::mojom::AppRuntimeBlinkDelegate>
          receiver,
      content::RenderFrameHost* rfh);

  void SetDelegate(WebViewDelegate* delegate);
  void SetBackHistoryKeyDisabled(bool disabled) {
    is_back_history_key_disabled_ = disabled;
  }

  // mojom::AppRuntimeWebViewHost implementation.
  void DidLoadingEnd(base::TimeDelta loading_end) override;
  void DidFirstPaint(base::TimeDelta first_paint) override;
  void DidFirstContentfulPaint(base::TimeDelta first_contentful_paint) override;
  void DidFirstImagePaint(base::TimeDelta first_image_paint) override;
  void DidFirstMeaningfulPaint(base::TimeDelta first_meaningful_paint) override;
  void DidClearWindowObject() override;
  void DidLargestContentfulPaint(
      base::TimeDelta largest_contentful_paint) override;
  void DidResumeDOM() override;
  void CreateNotification(const std::string& app_id,
                          const std::string& msg) override;

  // blink::mojom::AppRuntimeBlinkDelegate implementation.
  void IsBackHistoryKeyDisabled(
      IsBackHistoryKeyDisabledCallback callback) override;
  void DidHistoryBackOnTopPage() override;

 private:
  friend class content::WebContentsUserData<AppRuntimeWebViewHostImpl>;

  explicit AppRuntimeWebViewHostImpl(content::WebContents* web_contents);

  content::RenderFrameHostReceiverSet<mojom::AppRuntimeWebViewHost>
      webview_host_receivers_;
  content::RenderFrameHostReceiverSet<blink::mojom::AppRuntimeBlinkDelegate>
      blink_delegate_receivers_;

  raw_ptr<WebViewDelegate> webview_delegate_ = nullptr;
  bool is_back_history_key_disabled_ = false;

  std::unique_ptr<pal::NotificationManagerDelegate>
      notification_manager_delegate_;

  WEB_CONTENTS_USER_DATA_KEY_DECL();
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_BROWSER_APP_RUNTIME_WEBVIEW_HOST_IMPL_H_
