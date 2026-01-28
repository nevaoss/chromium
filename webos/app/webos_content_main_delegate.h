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

#ifndef WEBOS_APP_WEBOS_CONTENT_MAIN_DELEGATE_H_
#define WEBOS_APP_WEBOS_CONTENT_MAIN_DELEGATE_H_

#include <memory>
#include <optional>

#include "neva/app_runtime/app/app_runtime_main_delegate.h"
#include "webos/common/webos_content_client.h"

namespace webos {
class WebOSContentRendererClient;

class WebOSContentMainDelegate
    : public neva_app_runtime::AppRuntimeMainDelegate {
 public:
  WebOSContentMainDelegate();
  ~WebOSContentMainDelegate() override;

  // content::ContentMainDelegate implementation:
  std::optional<int> BasicStartupComplete() override;
  void PreSandboxStartup() override;
  void SetBasicStartupCallback(base::OnceClosure basic_startup_callback) {
    basic_startup_callback_ = std::move(basic_startup_callback);
  }
  void SetBrowserStartupCallback(base::OnceClosure startup_callback) {
    startup_callback_ = std::move(startup_callback);
  }

  content::ContentClient* CreateContentClient() override;
  content::ContentRendererClient* CreateContentRendererClient() override;

 protected:
  std::unique_ptr<WebOSContentClient> content_client_;
  std::unique_ptr<WebOSContentRendererClient> content_renderer_client_;
  base::OnceClosure basic_startup_callback_;
  base::OnceClosure startup_callback_;
};

}  // namespace webos

#endif  // WEBOS_APP_WEBOS_CONTENT_MAIN_DELEGATE_H_
