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

#include "neva/app_runtime/browser/app_runtime_web_contents_delegate.h"

#include "base/command_line.h"
#include "base/neva/base_switches.h"
#include "components/custom_handlers/protocol_handler.h"
#include "components/custom_handlers/protocol_handler_registry.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/file_select_listener.h"
#include "content/public/browser/web_contents.h"
#include "neva/app_runtime/browser/custom_handlers/app_runtime_protocol_handler_registry_factory.h"
#include "neva/pal_service/pal_platform_factory.h"
#include "neva/pal_service/public/notification_manager_delegate.h"
#include "third_party/blink/public/mojom/renderer_preferences.mojom.h"

namespace neva_app_runtime {

AppRuntimeWebContentsDelegate::AppRuntimeWebContentsDelegate() {
  if (base::CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kEnableNotificationForUnsupportedFeatures)) {
    notification_manager_delegate_ =
        pal::PlatformFactory::Get()->CreateNotificationManagerDelegate();
  }
}

AppRuntimeWebContentsDelegate::~AppRuntimeWebContentsDelegate() = default;

void AppRuntimeWebContentsDelegate::RunFileChooser(
    content::RenderFrameHost* render_frame_host,
    scoped_refptr<content::FileSelectListener> listener,
    const blink::mojom::FileChooserParams& params) {
  listener->FileSelectionCanceled();
  if (notification_manager_delegate_) {
    auto* web_contents =
        content::WebContents::FromRenderFrameHost(render_frame_host);

    if (!web_contents) {
      return;
    }

    std::string app_id =
        web_contents->GetMutableRendererPrefs()->application_id;
    notification_manager_delegate_->CreateToast(
        app_id, "File selection is not supported.");
  }
}

void AppRuntimeWebContentsDelegate::SetSSLCertErrorPolicy(
    SSLCertErrorPolicy policy) {}

SSLCertErrorPolicy AppRuntimeWebContentsDelegate::GetSSLCertErrorPolicy()
    const {
  return SSL_CERT_ERROR_POLICY_DEFAULT;
}

void AppRuntimeWebContentsDelegate::RegisterProtocolHandler(
    content::RenderFrameHost* requesting_frame,
    const std::string& protocol,
    const GURL& url,
    bool user_gesture) {
  if (!base::CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kEnableExternalProtocolsHandling)) {
    return;
  }

  content::BrowserContext* context = requesting_frame->GetBrowserContext();
  if (context->IsOffTheRecord()) {
    return;
  }

  custom_handlers::ProtocolHandler handler =
      custom_handlers::ProtocolHandler::CreateProtocolHandler(
          protocol, url, GetProtocolHandlerSecurityLevel(requesting_frame));

  // The parameters's normalization process defined in the spec has been already
  // applied in the WebContentImpl class, so at this point it shouldn't be
  // possible to create an invalid handler.
  // https://html.spec.whatwg.org/multipage/system-state.html#normalize-protocol-handler-parameters
  DCHECK(handler.IsValid());

  custom_handlers::ProtocolHandlerRegistry* registry =
      AppRuntimeProtocolHandlerRegistryFactory::GetForBrowserContext(context);
  if (registry->SilentlyHandleRegisterHandlerRequest(handler)) {
    return;
  }

  // TODO(neva): Enable user_gesture check logic.
  // Original code is as below:
#if 0
  if (!user_gesture && !windows_.empty()) {
    // TODO(jfernandez): This is not strictly needed, but we need a way to
    // inform the observers in browser tests that the request has been
    // cancelled, to avoid timeouts. Chrome just holds the handler as pending in
    // the PageContentSettingsDelegate, but we don't have such thing in the
    // Content Shell.
    registry->OnDenyRegisterProtocolHandler(handler);
    return;
  }
#endif

  // FencedFrames can not register to handle any protocols.
  if (requesting_frame->IsNestedWithinFencedFrame()) {
    registry->OnIgnoreRegisterProtocolHandler(handler);
    return;
  }

  // TODO(neva): Check registration mode to accept protocol handler.
  // Original code is as below:
  //  if (registry->registration_mode() ==
  //      custom_handlers::RphRegistrationMode::kAutoAccept) {
  //    registry->OnAcceptRegisterProtocolHandler(handler);
  //  }
  registry->OnAcceptRegisterProtocolHandler(handler);
}

void AppRuntimeWebContentsDelegate::UnregisterProtocolHandler(
    content::RenderFrameHost* requesting_frame,
    const std::string& protocol,
    const GURL& url,
    bool user_gesture) {
  if (!base::CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kEnableExternalProtocolsHandling)) {
    return;
  }

  // user_gesture will be used in case we decide to have confirmation bubble
  // for user while un-registering the handler.
  content::BrowserContext* context = requesting_frame->GetBrowserContext();
  if (context->IsOffTheRecord()) {
    return;
  }

  custom_handlers::ProtocolHandler handler =
      custom_handlers::ProtocolHandler::CreateProtocolHandler(
          protocol, url, GetProtocolHandlerSecurityLevel(requesting_frame));

  custom_handlers::ProtocolHandlerRegistry* registry =
      AppRuntimeProtocolHandlerRegistryFactory::GetForBrowserContext(context);
  registry->RemoveHandler(handler);
}

}  // namespace neva_app_runtime
