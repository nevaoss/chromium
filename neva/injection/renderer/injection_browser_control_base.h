// Copyright 2016-2019 LG Electronics, Inc.
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

#ifndef NEVA_INJECTION_RENDERER_INJECTION_BROWSER_CONTROL_BASE_H_
#define NEVA_INJECTION_RENDERER_INJECTION_BROWSER_CONTROL_BASE_H_

#include "content/common/content_export.h"
#include "mojo/public/cpp/bindings/associated_remote.h"
#include "neva/app_runtime/public/mojom/app_runtime_webview_controller.mojom.h"

namespace blink {

class WebLocalFrame;

}  // namespace blink

namespace injections {

class CONTENT_EXPORT InjectionBrowserControlBase {
 public:
  InjectionBrowserControlBase(blink::WebLocalFrame* web_local_frame);
  ~InjectionBrowserControlBase();

  void SendCommand(const std::string& command_name);
  void SendCommand(const std::string& command_name,
                   const std::vector<std::string>& arguments);
  std::string CallFunction(const std::string& function_name);
  std::string CallFunction(const std::string& function_name,
                           const std::vector<std::string>& arguments);
 private:
  mojo::AssociatedRemote<neva_app_runtime::mojom::AppRuntimeWebViewController>
      webview_controller_interface_;
};

}  // namespace injections

#endif  // NEVA_INJECTION_RENDERER_INJECTION_BROWSER_CONTROL_BASE_H_
