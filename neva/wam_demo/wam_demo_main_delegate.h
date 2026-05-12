// Copyright 2017 LG Electronics, Inc.
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

#ifndef NEVA_WAM_DEMO_WAM_DEMO_MAIN_DELEGATE_H_
#define NEVA_WAM_DEMO_WAM_DEMO_MAIN_DELEGATE_H_

#include <optional>

#include "build/build_config.h"
#include "content/public/common/main_function_params.h"
#include "neva/app_runtime/app/app_runtime_main_delegate.h"
#include "neva/wam_demo/wam_demo_export.h"

namespace wam_demo {

class WamDemoService;

class WAM_DEMO_EXPORT WamDemoMainDelegate
    : public neva_app_runtime::AppRuntimeMainDelegate {
 public:
  WamDemoMainDelegate(content::MainFunctionParams parameters);
  ~WamDemoMainDelegate() override;

  // content::ContentMainDelegate implementation:
  std::optional<int> BasicStartupComplete() override;

  void PreMainMessageLoopRun() override;

 private:
#if BUILDFLAG(IS_WEBOS)
  void ChangeUserIDGroupID();
#endif  // BUILDFLAG(IS_WEBOS)

  content::MainFunctionParams parameters_;
  std::unique_ptr<WamDemoService> service_;
};

}  // namespace wam_demo

#endif  // NEVA_WAM_DEMO_WAM_DEMO_MAIN_DELEGATE_H_
