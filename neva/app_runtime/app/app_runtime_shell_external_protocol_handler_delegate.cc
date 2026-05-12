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

#include "neva/app_runtime/app/app_runtime_shell_external_protocol_handler_delegate.h"

#include "base/functional/bind.h"
#include "base/functional/callback.h"
#include "base/task/thread_pool.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/web_contents.h"
#include "neva/app_runtime/app/app_runtime_page_contents.h"
#include "neva/app_runtime/app/app_runtime_shell_environment.h"

namespace neva_app_runtime {

AppRuntimeShellExternalProtocolHandlerDelegate::
    AppRuntimeShellExternalProtocolHandlerDelegate() = default;
AppRuntimeShellExternalProtocolHandlerDelegate::
    ~AppRuntimeShellExternalProtocolHandlerDelegate() = default;

void HandleExternalProtocolInUI(const GURL& url,
                                content::WebContents* web_contents) {
  ShellEnvironment::GetInstance()
      ->GetPageContentsFrom(web_contents)
      ->LoadURL(url.spec());
}

void AppRuntimeShellExternalProtocolHandlerDelegate::LaunchUrl(
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
  content::WebContents* web_contents = web_contents_getter.Run();
  content::GetUIThreadTaskRunner({})->PostTask(
      FROM_HERE, base::BindOnce(&HandleExternalProtocolInUI, url,
                                std::move(web_contents)));
}

}  // namespace neva_app_runtime
