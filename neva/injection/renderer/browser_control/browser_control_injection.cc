// Copyright 2018-2019 LG Electronics, Inc.
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

#include "neva/injection/renderer/browser_control/browser_control_injection.h"

#include <tuple>

#include "base/functional/bind.h"
#include "base/logging.h"
#include "gin/arguments.h"
#include "gin/function_template.h"
#include "gin/handle.h"
#include "third_party/blink/public/platform/scheduler/web_agent_group_scheduler.h"
#include "third_party/blink/public/web/blink.h"
#include "third_party/blink/public/web/web_local_frame.h"
#include "third_party/blink/public/web/web_script_source.h"

namespace injections {

gin::WrapperInfo BrowserControlInjection::kWrapperInfo = {
    gin::kEmbedderNativeGin};

const char BrowserControlInjection::kCallFunctionMethodName[] =
    "CallFunction";
const char BrowserControlInjection::kSendCommandMethodName[] =
    "SendCommand";

BrowserControlInjection::BrowserControlInjection(blink::WebLocalFrame* frame)
    : InjectionBrowserControlBase(frame) {
}

BrowserControlInjection::~BrowserControlInjection() {
}

void BrowserControlInjection::DoCallFunction(gin::Arguments* args) {
  std::string func_name;
  if (!args->GetNext(&func_name))
    return;

  std::string param;
  std::vector<std::string> func_arguments;
  while (args->GetNext(&param))
    func_arguments.push_back(param);

  args->Return(CallFunction(func_name, func_arguments));
}

void BrowserControlInjection::DoSendCommand(gin::Arguments* args) {
  std::string cmd_name;
  if (!args->GetNext(&cmd_name))
    return;

  std::string param;
  std::vector<std::string> cmd_arguments;
  while (args->GetNext(&param))
    cmd_arguments.push_back(param);

  SendCommand(cmd_name, cmd_arguments);
}

gin::ObjectTemplateBuilder BrowserControlInjection::GetObjectTemplateBuilder(
    v8::Isolate* isolate) {
  return gin::Wrappable<BrowserControlInjection>::
      GetObjectTemplateBuilder(isolate)
          .SetMethod(kCallFunctionMethodName,
                     &BrowserControlInjection::DoCallFunction)
          .SetMethod(kSendCommandMethodName,
                     &BrowserControlInjection::DoSendCommand);
}

// static
void BrowserControlInjection::Install(blink::WebLocalFrame* frame) {
  v8::Isolate* isolate = frame->GetAgentGroupScheduler()->Isolate();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = frame->MainWorldScriptContext();
  if (context.IsEmpty())
    return;

  v8::Local<v8::Object> global = context->Global();
  v8::Context::Scope context_scope(context);

  v8::Local<v8::Object> navigator;
  if (!gin::Converter<v8::Local<v8::Object>>::FromV8(
          isolate, global->Get(context, gin::StringToV8(isolate, "navigator"))
                       .ToLocalChecked(),
          &navigator))
    return;

  bool result;
  if (navigator->Has(context, gin::StringToV8(isolate, "browser_control"))
          .To(&result) &&
      result)
    return;

  auto obj = CreateObject(frame, isolate, navigator);
  std::ignore = obj;
}

void BrowserControlInjection::Uninstall(blink::WebLocalFrame* frame) {
  v8::Isolate* isolate = frame->GetAgentGroupScheduler()->Isolate();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = frame->MainWorldScriptContext();
  if (context.IsEmpty())
    return;

  v8::Local<v8::Object> global = context->Global();
  v8::Context::Scope context_scope(context);

  v8::Local<v8::Object> navigator;
  if (!gin::Converter<v8::Local<v8::Object>>::FromV8(
          isolate, global->Get(context, gin::StringToV8(isolate, "navigator"))
                       .ToLocalChecked(),
          &navigator))
    return;

  v8::Local<v8::String> browser_control_name =
    gin::StringToV8(isolate, "browser_control");
  bool result;
  if (navigator->Has(context, browser_control_name).To(&result) && result)
    std::ignore = navigator->Delete(context, browser_control_name);
}

// static
v8::MaybeLocal<v8::Object> BrowserControlInjection::CreateObject(
    blink::WebLocalFrame* frame,
    v8::Isolate* isolate,
    v8::Local<v8::Object> parent) {
  gin::Handle<BrowserControlInjection> browser_control =
      gin::CreateHandle(isolate, new BrowserControlInjection(frame));
  parent
      ->Set(frame->MainWorldScriptContext(),
            gin::StringToV8(isolate, "browser_control"), browser_control.ToV8())
      .Check();
  return browser_control->GetWrapper(isolate);
}

}  // namespace injections
