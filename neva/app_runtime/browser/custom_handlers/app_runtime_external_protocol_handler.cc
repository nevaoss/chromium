// Copyright 2024 LG Electronics, Inc.
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

#include "neva/app_runtime/browser/custom_handlers/app_runtime_external_protocol_handler.h"

#include "neva/app_runtime/browser/custom_handlers/app_runtime_external_protocol_handler_delegate.h"
#include "neva/app_runtime/browser/custom_handlers/app_runtime_protocol_handler_registry_delegate.h"
#include "neva/app_runtime/browser/custom_handlers/app_runtime_protocol_handler_registry_factory.h"

namespace neva_app_runtime {

// static
AppRuntimeExternalProtocolHandler*
AppRuntimeExternalProtocolHandler::GetIstance() {
  return base::Singleton<AppRuntimeExternalProtocolHandler>::get();
}

AppRuntimeExternalProtocolHandler::AppRuntimeExternalProtocolHandler() {}

AppRuntimeExternalProtocolHandler::~AppRuntimeExternalProtocolHandler() {}

void AppRuntimeExternalProtocolHandler::SetDelegate(
    AppRuntimeExternalProtocolHandlerDelegate* delegate) {
  delegate_.reset(delegate);
}

bool AppRuntimeExternalProtocolHandler::LaunchUrl(
    const GURL& url,
    content::WebContents::Getter web_contents_getter,
    content::FrameTreeNodeId frame_tree_node_id,
    content::NavigationUIData* navigation_data,
    bool is_primary_main_frame,
    bool is_in_fenced_frame_tree,
    network::mojom::WebSandboxFlags sandbox_flags,
    ui::PageTransition page_transition,
    bool has_user_gesture,
    const std::optional<url::Origin>& initiating_origin,
    content::RenderFrameHost* initiator_document,
    mojo::PendingRemote<network::mojom::URLLoaderFactory>* out_factory) {
  if (delegate_) {
    auto* protocol_handler_registry =
        AppRuntimeProtocolHandlerRegistryFactory::GetForBrowserContext(
            web_contents_getter.Run()->GetBrowserContext());
    GURL translated_url = protocol_handler_registry->Translate(url);

    if (!translated_url.is_empty()) {
      delegate_->LaunchUrl(translated_url, web_contents_getter,
                           frame_tree_node_id, navigation_data,
                           is_primary_main_frame, is_in_fenced_frame_tree,
                           sandbox_flags, page_transition, has_user_gesture,
                           initiating_origin, initiator_document, out_factory);
      return true;
    }
  }
  return false;
}

}  // namespace neva_app_runtime
