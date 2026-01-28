// Copyright 2018 LG Electronics, Inc.
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

#ifndef WEBOS_RENDERER_WEBOS_CONTENT_RENDERER_CLIENT_H_
#define WEBOS_RENDERER_WEBOS_CONTENT_RENDERER_CLIENT_H_

#include "neva/app_runtime/renderer/app_runtime_content_renderer_client.h"

namespace webos {

class WebOSContentRendererClient
    : public neva_app_runtime::AppRuntimeContentRendererClient {
 public:
  explicit WebOSContentRendererClient(bool override_error_page = true);
  void RenderFrameCreated(content::RenderFrame* render_frame) override;
  void PrepareErrorPage(content::RenderFrame* render_frame,
                        const blink::WebURLError& error,
                        const std::string& http_method,
                        content::mojom::AlternativeErrorPageOverrideInfoPtr
                            alternative_error_page_info,
                        std::string* error_html) override;
 private:
  bool override_error_page_;
};

}  // namespace webos

#endif  // WEBOS_RENDERER_WEBOS_CONTENT_RENDERER_CLIENT_H_
