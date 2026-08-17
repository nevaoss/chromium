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

#include "neva/app_runtime/renderer/app_runtime_content_renderer_client.h"

#include "base/memory/raw_ptr.h"
#include "content/public/common/url_constants.h"
#include "content/public/renderer/render_frame_observer.h"
#include "content/public/renderer/render_thread.h"
#include "content/public/renderer/render_thread_observer.h"
#include "net/base/filename_util.h"
#include "neva/app_runtime/app/app_runtime_main_delegate.h"
#include "neva/app_runtime/common/app_runtime_file_access_controller.h"
#include "neva/app_runtime/grit/app_runtime_network_error_resources.h"
#include "neva/app_runtime/public/webview_info.h"
#include "neva/app_runtime/renderer/app_runtime_localized_error.h"
#include "neva/app_runtime/renderer/app_runtime_page_load_timing_render_frame_observer.h"
#include "neva/app_runtime/renderer/app_runtime_render_frame_observer.h"
#include "neva/app_runtime/renderer/app_runtime_webview_observer.h"
#include "third_party/blink/public/mojom/fetch/fetch_api_request.mojom.h"
#include "third_party/blink/public/platform/web_url_error.h"
#include "third_party/blink/public/web/web_console_message.h"
#include "third_party/blink/public/web/web_local_frame.h"
#include "third_party/blink/public/web/web_view.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/webui/web_ui_util.h"

#if defined(USE_NEVA_CDM)
#include "components/cdm/renderer/neva/key_systems_util.h"
#endif

#if defined(USE_NEVA_CHROME_EXTENSIONS)
#include "base/process/current_process.h"
#include "extensions/common/switches.h"
#include "extensions/renderer/api/core_extensions_renderer_api_provider.h"
#include "extensions/renderer/extension_frame_helper.h"
#include "neva/extensions/renderer/api/neva_extensions_renderer_api_provider.h"
#include "third_party/blink/public/platform/scheduler/web_renderer_process_type.h"
#endif  // defined(USE_NEVA_CHROME_EXTENSIONS)

using blink::mojom::FetchCacheMode;

namespace neva_app_runtime {

class AppRuntimeRenderObserver : public content::RenderFrameObserver,
                                 public content::RenderThreadObserver {
 public:
  AppRuntimeRenderObserver(content::RenderFrame* render_frame,
                           AppRuntimeContentRendererClient* renderer_client)
      : content::RenderFrameObserver(render_frame),
        renderer_client_(renderer_client) {
    content::RenderThread::Get()->AddObserver(this);
  }

  ~AppRuntimeRenderObserver() override {
    content::RenderThread::Get()->RemoveObserver(this);
  }

  void OnDestruct() override {
    if (renderer_client_)
      renderer_client_->DestructObserver();
  }

  void NetworkStateChanged(bool online) override {
    if (online) {
      if (render_frame() && render_frame()->GetWebFrame()) {
        render_frame()->GetWebFrame()->StartReload(
            blink::WebFrameLoadType::kReload);
      }
      OnDestruct();
    }
  }

 private:
  raw_ptr<AppRuntimeContentRendererClient> renderer_client_;
};

AppRuntimeContentRendererClient::AppRuntimeContentRendererClient() {}

AppRuntimeContentRendererClient::~AppRuntimeContentRendererClient() {}

void AppRuntimeContentRendererClient::RenderFrameCreated(
    content::RenderFrame* render_frame) {
#if defined(USE_NEVA_CHROME_EXTENSIONS)
  extensions::Dispatcher* dispatcher =
      extensions_renderer_client_->dispatcher();
  // ExtensionFrameHelper destroys itself when the RenderFrame is destroyed.
  new extensions::ExtensionFrameHelper(render_frame, dispatcher);
  dispatcher->OnRenderFrameCreated(render_frame);
#endif  // defined(USE_NEVA_CHROME_EXTENSIONS)

  // AppRuntimeRenderFrameObserver destroys itself when the RenderFrame is
  // destroyed.
  new AppRuntimeRenderFrameObserver(render_frame);
  // Only attach AppRuntimePageLoadTimingRenderFrameObserver to the main frame,
  // since we only want to observe page load timing for the main frame.
  if (render_frame->IsMainFrame()) {
    new AppRuntimePageLoadTimingRenderFrameObserver(render_frame);
  }
}

void AppRuntimeContentRendererClient::PrepareErrorPage(
    content::RenderFrame* render_frame,
    const blink::WebURLError& error,
    const std::string& http_method,
    content::mojom::AlternativeErrorPageOverrideInfoPtr
        alternative_error_page_info,
    std::string* error_html) {
  if (error_html) {
    error_html->clear();

    // Resource will change to net error specific page
    int resource_id = IDR_APP_RUNTIME_NETWORK_ERROR_PAGE;
    const std::string template_html =
        ui::ResourceBundle::GetSharedInstance().LoadDataResourceString(
            resource_id);
    if (template_html.empty()) {
      LOG(ERROR) << "unable to load template.";
    } else {
      base::DictValue error_strings;
      AppRuntimeLocalizedError::GetErrorStrings(error.reason(), error_strings);
      // "t" is the id of the template's root node.
      *error_html = webui::GetLocalizedHtml(template_html, error_strings);
    }

    render_observer_.reset(new AppRuntimeRenderObserver(render_frame, this));
  }
}

void AppRuntimeContentRendererClient::WillSendRequest(
    blink::WebLocalFrame* frame,
    ui::PageTransition transition_type,
    const blink::WebURL& upstream_url,
    const blink::WebURL& target_url,
    const net::SiteForCookies& site_for_cookies,
    const url::Origin* initiator_origin,
    GURL* new_url) {
  // Ignore non-file scheme requests
  if (!static_cast<GURL>(target_url).SchemeIsFile())
    return;

  // Ignore file scheme requests from non-file scheme origins granted
  // with allow_local_resource_load permission
  if (!initiator_origin->GetURL().SchemeIsFile())
    return;

  const AppRuntimeFileAccessController* file_access_controller =
      GetFileAccessController();

  if (file_access_controller) {
    // Ignore navigations since they are handled on browser side
    if (!ui::PageTransitionTypeIncludingQualifiersIs(transition_type,
                                                     ui::PAGE_TRANSITION_FIRST))
      return;

    base::FilePath file_path;
    if (!net::FileURLToFilePath(GURL(target_url), &file_path) ||
        !file_access_controller->IsAccessAllowed(file_path, webview_info_)) {
      blink::WebConsoleMessage error_msg;
      error_msg.level = blink::mojom::ConsoleMessageLevel::kError;
      error_msg.text = blink::WebString::FromAscii(
          "Access is blocked to resource: " + target_url.GetString().Ascii());
      frame->AddMessageToConsole(error_msg);

      // Redirect to unreachable URL (throws net::ERR_UNKNOWN_URL_SCHEME
      // to console)
      *new_url = GURL(content::kUnreachableWebDataURL);
    }
  }
}

void AppRuntimeContentRendererClient::SetWebViewInfo(
    const std::string& app_path, const std::string& trust_level) {
  webview_info_.app_path = app_path;
  webview_info_.trust_level = trust_level;
}

void AppRuntimeContentRendererClient::DestructObserver() {
  render_observer_.reset();
}

#if defined(USE_NEVA_CDM)
void AppRuntimeContentRendererClient::GetSupportedKeySystems(
    media::GetSupportedKeySystemsCB cb) {
  cdm::AddSupportedKeySystems(std::move(cb));
}
#endif

void AppRuntimeContentRendererClient::WebViewCreated(
    blink::WebView* web_view,
    bool was_created_by_renderer,
    const url::Origin* outermost_origin) {
  // Owns itself; deleted through OnDestruct() when the WebView goes away.
  new AppRuntimeWebViewObserver(web_view);
#if defined(USE_NEVA_CHROME_EXTENSIONS)
  extensions_renderer_client_->WebViewCreated(web_view, outermost_origin);
#endif  // defined(USE_NEVA_CHROME_EXTENSIONS)
}

#if defined(USE_NEVA_CHROME_EXTENSIONS)
void AppRuntimeContentRendererClient::RenderThreadStarted() {
  extensions_client_.reset(new neva::NevaExtensionsClient);
  extensions::ExtensionsClient::Set(extensions_client_.get());

  extensions_renderer_client_.reset(new neva::NevaExtensionsRendererClient);
  extensions_renderer_client_->AddAPIProvider(
      std::make_unique<extensions::CoreExtensionsRendererAPIProvider>());
  extensions_renderer_client_->AddAPIProvider(
      std::make_unique<neva::NevaExtensionsRendererAPIProvider>());
  extensions_renderer_client_->RenderThreadStarted();
  extensions::ExtensionsRendererClient::Set(extensions_renderer_client_.get());

  const bool is_extension = base::CommandLine::ForCurrentProcess()->HasSwitch(
      extensions::switches::kExtensionProcess);

  if (is_extension) {
    // The process name was set to "Renderer" in RendererMain(). Update it to
    // "Extension Renderer" to highlight that it's hosting an extension.
    base::CurrentProcess::GetInstance().SetProcessType(
        base::CurrentProcessType::PROCESS_RENDERER_EXTENSION);
  }
}

void AppRuntimeContentRendererClient::RunScriptsAtDocumentStart(
    content::RenderFrame* render_frame) {
  extensions_renderer_client_->dispatcher()->RunScriptsAtDocumentStart(
      render_frame);
}

void AppRuntimeContentRendererClient::RunScriptsAtDocumentEnd(
    content::RenderFrame* render_frame) {
  extensions_renderer_client_->dispatcher()->RunScriptsAtDocumentEnd(
      render_frame);
}

void AppRuntimeContentRendererClient::
    DidInitializeServiceWorkerContextOnWorkerThread(
        blink::WebServiceWorkerContextProxy* context_proxy,
        const GURL& service_worker_scope,
        const GURL& script_url) {
  extensions_renderer_client_->dispatcher()
      ->DidInitializeServiceWorkerContextOnWorkerThread(
          context_proxy, service_worker_scope, script_url);
}

void AppRuntimeContentRendererClient::WillEvaluateServiceWorkerOnWorkerThread(
    blink::WebServiceWorkerContextProxy* context_proxy,
    v8::Local<v8::Context> v8_context,
    int64_t service_worker_version_id,
    const GURL& service_worker_scope,
    const GURL& script_url,
    const blink::ServiceWorkerToken& service_worker_token) {
  extensions_renderer_client_->dispatcher()
      ->WillEvaluateServiceWorkerOnWorkerThread(
          context_proxy, v8_context, service_worker_version_id,
          service_worker_scope, script_url, service_worker_token);
}

void AppRuntimeContentRendererClient::
    DidStartServiceWorkerContextOnWorkerThread(
        int64_t service_worker_version_id,
        const GURL& service_worker_scope,
        const GURL& script_url,
        const blink::ServiceWorkerToken& service_worker_token) {
  extensions_renderer_client_->dispatcher()
      ->DidStartServiceWorkerContextOnWorkerThread(
          service_worker_version_id, service_worker_scope, script_url,
          service_worker_token);
}

void AppRuntimeContentRendererClient::
    WillDestroyServiceWorkerContextOnWorkerThread(
        v8::Local<v8::Context> context,
        int64_t service_worker_version_id,
        const GURL& service_worker_scope,
        const GURL& script_url,
        const blink::ServiceWorkerToken& service_worker_token) {
  extensions_renderer_client_->dispatcher()
      ->WillDestroyServiceWorkerContextOnWorkerThread(
          context, service_worker_version_id, service_worker_scope, script_url,
          service_worker_token);
}
#endif  // defined(USE_NEVA_CHROME_EXTENSIONS)

}  // namespace neva_app_runtime
