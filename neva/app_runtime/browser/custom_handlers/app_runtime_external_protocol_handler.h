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

#ifndef NEVA_APP_RUNTIME_BROWSER_CUSTOM_HANDLERS_APP_RUNTIME_EXTERNAL_PROTOCOL_HANDLER_H_
#define NEVA_APP_RUNTIME_BROWSER_CUSTOM_HANDLERS_APP_RUNTIME_EXTERNAL_PROTOCOL_HANDLER_H_

#include <memory>

#include "base/memory/singleton.h"
#include "content/public/browser/web_contents.h"
#include "ui/base/page_transition_types.h"

class GURL;

namespace neva_app_runtime {

class AppRuntimeExternalProtocolHandlerDelegate;

class AppRuntimeExternalProtocolHandler {
 public:
  static AppRuntimeExternalProtocolHandler* GetIstance();
  void SetDelegate(AppRuntimeExternalProtocolHandlerDelegate* delegate);

  bool LaunchUrl(
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
      mojo::PendingRemote<network::mojom::URLLoaderFactory>* out_factory);

 private:
  friend struct base::DefaultSingletonTraits<AppRuntimeExternalProtocolHandler>;

  AppRuntimeExternalProtocolHandler();
  ~AppRuntimeExternalProtocolHandler();

  std::unique_ptr<AppRuntimeExternalProtocolHandlerDelegate> delegate_;
};

}  // namespace neva_app_runtime

#endif  //  NEVA_APP_RUNTIME_BROWSER_CUSTOM_HANDLERS_APP_RUNTIME_EXTERNAL_PROTOCOL_HANDLER_H_
