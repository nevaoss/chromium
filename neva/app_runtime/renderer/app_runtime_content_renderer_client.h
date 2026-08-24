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

#ifndef NEVA_APP_RUNTIME_RENDERER_APP_RUNTIME_CONTENT_RENDERER_CLIENT_H_
#define NEVA_APP_RUNTIME_RENDERER_APP_RUNTIME_CONTENT_RENDERER_CLIENT_H_

#include "content/public/renderer/content_renderer_client.h"
#include "neva/app_runtime/public/webview_info.h"

#if defined(USE_NEVA_CHROME_EXTENSIONS)
#include "neva/extensions/common/neva_extensions_client.h"
#include "neva/extensions/renderer/neva_extensions_renderer_client.h"
#endif  // defined(USE_NEVA_CHROME_EXTENSIONS)

namespace neva_app_runtime {

class AppRuntimeRenderObserver;

class AppRuntimeContentRendererClient : public content::ContentRendererClient {
 public:
  AppRuntimeContentRendererClient();
  ~AppRuntimeContentRendererClient() override;
  AppRuntimeContentRendererClient(const AppRuntimeContentRendererClient&) =
      delete;
  AppRuntimeContentRendererClient& operator=(
      const AppRuntimeContentRendererClient&) = delete;

  void RenderFrameCreated(content::RenderFrame* render_frame) override;

  void PrepareErrorPage(content::RenderFrame* render_frame,
                        const blink::WebURLError& error,
                        const std::string& http_method,
                        content::mojom::AlternativeErrorPageOverrideInfoPtr
                            alternative_error_page_info,
                        std::string* error_html) override;

  void WillSendRequest(blink::WebLocalFrame* frame,
                       ui::PageTransition transition_type,
                       const blink::WebURL& upstream_url,
                       const blink::WebURL& target_url,
                       const net::SiteForCookies& site_for_cookies,
                       const url::Origin* initiator_origin,
                       GURL* new_url) override;

  void SetWebViewInfo(const std::string& app_path,
                      const std::string& trust_level);

#if defined(USE_NEVA_CDM)
  void GetSupportedKeySystems(media::GetSupportedKeySystemsCB cb) override;
#endif

  void WebViewCreated(blink::WebView* web_view,
                      bool was_created_by_renderer,
                      const url::Origin* outermost_origin) override;
#if defined(USE_NEVA_CHROME_EXTENSIONS)
  void RenderThreadStarted() override;
  void RunScriptsAtDocumentStart(content::RenderFrame* render_frame) override;
  void RunScriptsAtDocumentEnd(content::RenderFrame* render_frame) override;
  bool AllowScriptExtensionForServiceWorker(
      const url::Origin& script_origin);
  void DidInitializeServiceWorkerContextOnWorkerThread(
      blink::WebServiceWorkerContextProxy* context_proxy,
      const GURL& service_worker_scope,
      const GURL& script_url) override;
  void WillEvaluateServiceWorkerOnWorkerThread(
      blink::WebServiceWorkerContextProxy* context_proxy,
      v8::Local<v8::Context> v8_context,
      int64_t service_worker_version_id,
      const GURL& service_worker_scope,
      const GURL& script_url,
      const blink::ServiceWorkerToken& service_worker_token) override;
  void DidStartServiceWorkerContextOnWorkerThread(
      int64_t service_worker_version_id,
      const GURL& service_worker_scope,
      const GURL& script_url,
      const blink::ServiceWorkerToken& service_worker_token) override;
  void WillDestroyServiceWorkerContextOnWorkerThread(
      v8::Local<v8::Context> context,
      int64_t service_worker_version_id,
      const GURL& service_worker_scope,
      const GURL& script_url,
      const blink::ServiceWorkerToken& service_worker_token) override;
#endif  // defined(USE_NEVA_CHROME_EXTENSIONS)

  void DestructObserver();

 private:
  std::unique_ptr<AppRuntimeRenderObserver> render_observer_;

  WebViewInfo webview_info_;

#if defined(USE_NEVA_CHROME_EXTENSIONS)
  std::unique_ptr<neva::NevaExtensionsClient> extensions_client_;
  std::unique_ptr<neva::NevaExtensionsRendererClient>
      extensions_renderer_client_;
#endif  // defined(USE_NEVA_CHROME_EXTENSIONS)
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_RENDERER_APP_RUNTIME_CONTENT_RENDERER_CLIENT_H_
