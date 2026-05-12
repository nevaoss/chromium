// Copyright 2023 LG Electronics, Inc.
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

#ifndef WEBOS_BROWSER_SHELL_BROWSER_SHELL_WEBOS_MAIN_DELEGATE_H_
#define WEBOS_BROWSER_SHELL_BROWSER_SHELL_WEBOS_MAIN_DELEGATE_H_

#include <memory>
#include <optional>

#include "neva/browser_shell/app/browser_shell_main_delegate.h"
#include "webos/common/webos_content_client.h"
#include "webos/common/webos_export.h"

namespace content {
class ContentClient;
class ContentRendererClient;
}  // namespace content

namespace webos {

class WEBOS_EXPORT BrowserShellWebOSMainDelegate
    : public browser_shell::BrowserShellMainDelegate {
 public:
  BrowserShellWebOSMainDelegate(content::MainFunctionParams parameters);
  ~BrowserShellWebOSMainDelegate() override;

  void PreSandboxStartup() override;

  std::optional<int> BasicStartupComplete() override;
  content::ContentClient* CreateContentClient() override;
  content::ContentRendererClient* CreateContentRendererClient() override;

 private:
  void ChangeUserIDGroupID();

  std::unique_ptr<WebOSContentClient> content_client_;
  std::unique_ptr<content::ContentRendererClient> content_renderer_client_;
};

}  // namespace webos

#endif  // WEBOS_BROWSER_SHELL_BROWSER_SHELL_WEBOS_MAIN_DELEGATE_H_
