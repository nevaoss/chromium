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

#include "neva/injection/renderer/injection_browser_control_base.h"

#include <string>

#include "base/check.h"
#include "content/public/renderer/render_frame.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_provider.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_registry.h"
#include "third_party/blink/public/web/web_local_frame.h"
#include "third_party/blink/public/web/web_script_source.h"

namespace injections {

InjectionBrowserControlBase::InjectionBrowserControlBase(
    blink::WebLocalFrame* frame) {
  auto* render_frame = content::RenderFrame::FromWebFrame(frame);
  CHECK(render_frame);
  render_frame->GetRemoteAssociatedInterfaces()->GetInterface(
      &webview_controller_interface_);
}

InjectionBrowserControlBase::~InjectionBrowserControlBase() = default;

void InjectionBrowserControlBase::SendCommand(const std::string& command_name) {
  SendCommand(command_name, std::vector<std::string>());
}

void InjectionBrowserControlBase::SendCommand(
    const std::string& command_name,
    const std::vector<std::string>& arguments) {
  webview_controller_interface_->SendCommand(command_name, arguments);
}

std::string InjectionBrowserControlBase::CallFunction(
    const std::string& function_name) {
  return CallFunction(function_name, std::vector<std::string>());
}

std::string InjectionBrowserControlBase::CallFunction(
    const std::string& function_name,
    const std::vector<std::string>& arguments) {
  std::string result;
  webview_controller_interface_->CallFunction(
      function_name, arguments, &result);
  return result;
}

}  // namespace injections
