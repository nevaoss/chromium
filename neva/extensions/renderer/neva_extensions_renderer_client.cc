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

#include "neva/extensions/renderer/neva_extensions_renderer_client.h"

#include <vector>

#include "content/public/renderer/render_thread.h"
#include "extensions/common/constants.h"
#include "extensions/common/extension.h"
#include "extensions/common/manifest_handlers/background_info.h"
#include "extensions/renderer/extension_web_view_helper.h"
#include "extensions/renderer/extensions_renderer_api_provider.h"

namespace neva {

NevaExtensionsRendererClient::NevaExtensionsRendererClient() = default;

NevaExtensionsRendererClient::~NevaExtensionsRendererClient() = default;

bool NevaExtensionsRendererClient::IsIncognitoProcess() const {
  // app_shell doesn't support off-the-record contexts.
  return false;
}

int NevaExtensionsRendererClient::GetLowestIsolatedWorldId() const {
  // app_shell doesn't need to reserve world IDs for anything other than
  // extensions, so we always return 1. Note that 0 is reserved for the global
  // world.
  return 1;
}

void NevaExtensionsRendererClient::WebViewCreated(
    blink::WebView* web_view,
    const url::Origin* outermost_origin) {
  new extensions::ExtensionWebViewHelper(web_view, outermost_origin);
}

}  // namespace neva
