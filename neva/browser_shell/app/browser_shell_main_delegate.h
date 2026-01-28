// Copyright 2021 LG Electronics, Inc.
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

#ifndef NEVA_BROWSER_SHELL_APP_BROWSER_SHELL_MAIN_DELEGATE_H_
#define NEVA_BROWSER_SHELL_APP_BROWSER_SHELL_MAIN_DELEGATE_H_

#include <memory>
#include <optional>

#include "content/public/common/main_function_params.h"
#include "neva/app_runtime/app/app_runtime_main_delegate.h"
#include "neva/browser_shell/common/browser_shell_export.h"

namespace neva_app_runtime {
class Shell;
}

namespace browser_shell {

class PlatformLanguage;
class PlatformRegistration;

class BROWSER_SHELL_EXPORT BrowserShellMainDelegate
    : public neva_app_runtime::AppRuntimeMainDelegate {
 public:
  BrowserShellMainDelegate(content::MainFunctionParams parameters);
  ~BrowserShellMainDelegate() override;

  void PreMainMessageLoopRun() override;
  void WillRunMainMessageLoop(
      std::unique_ptr<base::RunLoop>& run_loop) override;
  std::optional<int> BasicStartupComplete() override;

 private:
  content::MainFunctionParams parameters_;
  std::unique_ptr<PlatformLanguage> platform_language_;
  std::unique_ptr<PlatformRegistration> platform_registration_;
};

}  // namespace browser_shell

#endif  // NEVA_BROWSER_SHELL_APP_BROWSER_SHELL_MAIN_DELEGATE_H_
