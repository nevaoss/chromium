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

#include "webos/renderer/webos_network_error_helper.h"

#include "content/public/common/url_constants.h"
#include "content/renderer/render_frame_impl.h"
#include "third_party/blink/public/platform/web_url.h"
#include "third_party/blink/public/web/web_document.h"
#include "third_party/blink/public/web/web_local_frame.h"

#if defined(ENABLE_NETWORK_ERROR_PAGE_CONTROLLER_WEBAPI)
#include "neva/injection/public/renderer/network_error_page_controller_webapi.h"
#endif

namespace webos {

WebOSNetworkErrorHelper::WebOSNetworkErrorHelper(
    content::RenderFrame* render_frame)
    : content::RenderFrameObserver(render_frame),
      content::RenderFrameObserverTracker<WebOSNetworkErrorHelper>(
          render_frame) {}

void WebOSNetworkErrorHelper::DidCommitProvisionalLoad(
    ui::PageTransition transition) {}

void WebOSNetworkErrorHelper::OnDestruct() {
#if defined(ENABLE_NETWORK_ERROR_PAGE_CONTROLLER_WEBAPI)
  if (render_frame()) {
    blink::WebLocalFrame* web_local_frame = render_frame()->GetWebFrame();
    if (web_local_frame)
      injections::NetworkErrorPageControllerWebAPI::Uninstall(web_local_frame);
  }
#else
  NOTIMPLEMEMTED();
#endif
  delete this;
}

void WebOSNetworkErrorHelper::DidFinishLoad() {
#if defined(ENABLE_NETWORK_ERROR_PAGE_CONTROLLER_WEBAPI)
  if (!render_frame() || !render_frame()->IsMainFrame())
    return;

  blink::WebLocalFrame* web_local_frame = render_frame()->GetWebFrame();
  if (!web_local_frame)
    return;

  if (!web_local_frame->GetDocument().Url().IsValid() ||
      web_local_frame->GetDocument().Url().GetString() !=
          content::kUnreachableWebDataURL)
    return;

  injections::NetworkErrorPageControllerWebAPI::Install(web_local_frame,
                                                        std::string());
#else
  NOTIMPLEMEMTED();
#endif
}

}  // namespace webos
