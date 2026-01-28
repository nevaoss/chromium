// Copyright 2019 LG Electronics, Inc.
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

#include "neva/app_runtime/renderer/app_runtime_injection_loader.h"

#include "base/command_line.h"
#include "content/public/common/content_neva_switches.h"
#include "neva/injection/public/common/webapi_names.h"

#if defined(ENABLE_BROWSER_CONTROL_WEBAPI)
#include "neva/injection/public/renderer/browser_control_webapi.h"
#endif

#if defined(ENABLE_SAMPLE_WEBAPI)
#include "neva/injection/public/renderer/sample_webapi.h"
#endif

namespace neva_app_runtime {

InjectionLoader::InjectionLoader() {
  using namespace injections;
#if defined(ENABLE_BROWSER_CONTROL_WEBAPI)
  if (base::CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kEnableBrowserControlInjection))
    Add(std::string(injections::webapi::kBrowserControl), std::string("{}"));
#endif  // ENABLE_BROWSER_CONTROL_WEBAPI

#if defined(ENABLE_SAMPLE_WEBAPI)
  if (base::CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kEnableSampleInjection))
    Add(std::string(injections::webapi::kSample), std::string("{}"));
#endif  // ENABLE_SAMPLE_WEBAPI
}

InjectionLoader::~InjectionLoader() {}

void InjectionLoader::Add(std::string name, std::string arguments_json) {
  using namespace injections;
  if (injections_.insert(name).second) {
    InjectionContext context;
    context.arguments_json = std::move(arguments_json);
    if (injections::GetInjectionInstallAPI(name, &context.api))
      injections_contexts_.push_back(context);
  }
}

void InjectionLoader::Load(blink::WebLocalFrame* frame) {
  for (auto& context : injections_contexts_) {
    context.frame = frame;
    (*context.api.install_func)(frame, context.arguments_json);
  }
}

void InjectionLoader::Unload() {
  for (auto rit = injections_contexts_.crbegin();
       rit != injections_contexts_.crend(); ++rit) {
    if (rit->frame)
      (*rit->api.uninstall_func)(rit->frame);
  }
  injections_.clear();
  injections_contexts_.clear();
}

}  // namespace neva_app_runtime
