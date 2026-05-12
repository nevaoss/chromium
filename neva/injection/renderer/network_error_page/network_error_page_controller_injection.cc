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

#include "neva/injection/renderer/network_error_page/network_error_page_controller_injection.h"

#include "base/functional/bind.h"
#include "gin/arguments.h"
#include "gin/handle.h"
#include "neva/injection/renderer/grit/injection_resources.h"
#include "neva/pal_service/public/mojom/constants.mojom.h"
#include "third_party/blink/public/common/thread_safe_browser_interface_broker_proxy.h"
#include "third_party/blink/public/platform/platform.h"
#include "third_party/blink/public/platform/scheduler/web_agent_group_scheduler.h"
#include "third_party/blink/public/web/blink.h"
#include "third_party/blink/public/web/web_local_frame.h"
#include "third_party/blink/public/web/web_script_source.h"

namespace injections {

const char NetworkErrorPageControllerInjection::kExposedName[] =
    "errorPageController";

gin::WrapperInfo NetworkErrorPageControllerInjection::kWrapperInfo = {
    gin::kEmbedderNativeGin};

NetworkErrorPageControllerInjection::NetworkErrorPageControllerInjection() {
  blink::Platform::Current()->GetBrowserInterfaceBroker()->GetInterface(
      controller_.BindNewPipeAndPassReceiver());
}

NetworkErrorPageControllerInjection::~NetworkErrorPageControllerInjection() {}

bool NetworkErrorPageControllerInjection::SettingsButtonClick(int target_id) {
  controller_->LaunchNetworkSettings(target_id);
  return true;
}

void NetworkErrorPageControllerInjection::CloseShellIfNeeded() {
  controller_->CloseShellIfNeeded();
}

gin::ObjectTemplateBuilder
NetworkErrorPageControllerInjection::GetObjectTemplateBuilder(
    v8::Isolate* isolate) {
  return gin::Wrappable<NetworkErrorPageControllerInjection>::
      GetObjectTemplateBuilder(isolate)
          .SetMethod("settingsButtonClick",
                     &NetworkErrorPageControllerInjection::SettingsButtonClick)
          .SetMethod("closeShellIfNeeded",
                     &NetworkErrorPageControllerInjection::CloseShellIfNeeded);
}

void NetworkErrorPageControllerInjection::Install(blink::WebLocalFrame* frame) {
  v8::Isolate* isolate = frame->GetAgentGroupScheduler()->Isolate();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = frame->MainWorldScriptContext();
  if (context.IsEmpty())
    return;

  v8::Context::Scope context_scope(context);

  gin::Handle<NetworkErrorPageControllerInjection> controller =
      gin::CreateHandle(isolate, new NetworkErrorPageControllerInjection());
  if (controller.IsEmpty())
    return;

  v8::Local<v8::Object> global = context->Global();
  global
      ->Set(context,
            gin::StringToV8(isolate,
                            NetworkErrorPageControllerInjection::kExposedName),
            controller.ToV8())
      .ToChecked();
}

void NetworkErrorPageControllerInjection::Uninstall(
    blink::WebLocalFrame* frame) {
  v8::Isolate* isolate = frame->GetAgentGroupScheduler()->Isolate();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = frame->MainWorldScriptContext();
  if (context.IsEmpty())
    return;

  v8::Local<v8::Object> global = context->Global();
  v8::Context::Scope context_scope(context);
  v8::Local<v8::String> key = gin::StringToV8(
      isolate, NetworkErrorPageControllerInjection::kExposedName);

  if (!global->Has(context, key).FromMaybe(true))
    return;

  v8::Local<v8::Object> controller;
  if (gin::Converter<v8::Local<v8::Object>>::FromV8(
          isolate, global->Get(context, key).ToLocalChecked(), &controller)) {
    global->Delete(context, key);
  }
}

}  // namespace injections
