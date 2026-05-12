// Copyright 2016 LG Electronics, Inc.
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

#include "neva/app_runtime/browser/app_runtime_webview_host_impl.h"

#include "neva/app_runtime/public/webview_delegate.h"
#include "neva/pal_service/pal_platform_factory.h"
#include "neva/pal_service/public/notification_manager_delegate.h"

namespace neva_app_runtime {

AppRuntimeWebViewHostImpl::~AppRuntimeWebViewHostImpl() {}

// static
void AppRuntimeWebViewHostImpl::BindAppRuntimeWebViewHost(
    mojo::PendingAssociatedReceiver<mojom::AppRuntimeWebViewHost> receiver,
    content::RenderFrameHost* rfh) {
  auto* web_contents = content::WebContents::FromRenderFrameHost(rfh);
  if (!web_contents)
    return;

  auto* webview_host = AppRuntimeWebViewHostImpl::FromWebContents(web_contents);
  if (!webview_host)
    return;

  webview_host->webview_host_receivers_.Bind(rfh, std::move(receiver));
}

// static
void AppRuntimeWebViewHostImpl::BindAppRuntimeBlinkDelegate(
    mojo::PendingAssociatedReceiver<blink::mojom::AppRuntimeBlinkDelegate>
        receiver,
    content::RenderFrameHost* rfh) {
  auto* web_contents = content::WebContents::FromRenderFrameHost(rfh);
  if (!web_contents)
    return;

  auto* webview_host = AppRuntimeWebViewHostImpl::FromWebContents(web_contents);
  if (!webview_host)
    return;

  webview_host->blink_delegate_receivers_.Bind(rfh, std::move(receiver));
}

void AppRuntimeWebViewHostImpl::SetDelegate(WebViewDelegate* delegate) {
  webview_delegate_ = delegate;
}

void AppRuntimeWebViewHostImpl::DidLoadingEnd(base::TimeDelta loading_end) {
  if (webview_delegate_) {
    webview_delegate_->DidLoadingEnd(loading_end.InMicroseconds());
    // FIXME(neva): This workaround temporarily fixes showing of WebApp window.
    webview_delegate_->DidSwapCompositorFrame();
  }
}

void AppRuntimeWebViewHostImpl::DidFirstPaint(base::TimeDelta first_paint) {
  if (webview_delegate_) {
    webview_delegate_->DidFirstPaint(first_paint.InMicroseconds());
    webview_delegate_->DidSwapCompositorFrame();
  }
}

void AppRuntimeWebViewHostImpl::DidFirstContentfulPaint(
    base::TimeDelta first_contentful_paint) {
  if (webview_delegate_)
    webview_delegate_->DidFirstContentfulPaint(
        first_contentful_paint.InMicroseconds());
}

void AppRuntimeWebViewHostImpl::DidFirstImagePaint(
    base::TimeDelta first_image_paint) {
  if (webview_delegate_)
    webview_delegate_->DidFirstImagePaint(first_image_paint.InMicroseconds());
}

void AppRuntimeWebViewHostImpl::DidFirstMeaningfulPaint(
    base::TimeDelta first_meaningful_paint) {
  if (webview_delegate_)
    webview_delegate_->DidFirstMeaningfulPaint(
        first_meaningful_paint.InMicroseconds());
}

void AppRuntimeWebViewHostImpl::DidClearWindowObject() {
  if (webview_delegate_)
    webview_delegate_->DidClearWindowObject();
}

void AppRuntimeWebViewHostImpl::DidHistoryBackOnTopPage() {
  if (webview_delegate_)
    webview_delegate_->DidHistoryBackOnTopPage();
}

void AppRuntimeWebViewHostImpl::IsBackHistoryKeyDisabled(
    IsBackHistoryKeyDisabledCallback callback) {
  if (webview_delegate_)
    std::move(callback).Run(is_back_history_key_disabled_);
}

void AppRuntimeWebViewHostImpl::DidLargestContentfulPaint(
    base::TimeDelta largest_contentful_paint) {
  if (webview_delegate_)
    webview_delegate_->DidLargestContentfulPaint(
        largest_contentful_paint.InMicroseconds());
}

void AppRuntimeWebViewHostImpl::DidResumeDOM() {
  if (webview_delegate_)
    webview_delegate_->DidResumeDOM();
}

void AppRuntimeWebViewHostImpl::CreateNotification(const std::string& app_id,
                                                   const std::string& msg) {
  if (notification_manager_delegate_) {
    notification_manager_delegate_->CreateToast(app_id, msg);
  }
}

AppRuntimeWebViewHostImpl::AppRuntimeWebViewHostImpl(
    content::WebContents* web_contents)
    : content::WebContentsUserData<AppRuntimeWebViewHostImpl>(*web_contents),
      webview_host_receivers_(web_contents, this),
      blink_delegate_receivers_(web_contents, this),
      notification_manager_delegate_(
          pal::PlatformFactory::Get()->CreateNotificationManagerDelegate()) {}

WEB_CONTENTS_USER_DATA_KEY_IMPL(AppRuntimeWebViewHostImpl);

}  // namespace neva_app_runtime
