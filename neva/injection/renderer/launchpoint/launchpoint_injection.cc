// Copyright 2025 LG Electronics, Inc.
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

#include "neva/injection/renderer/launchpoint/launchpoint_injection.h"

#include <tuple>

#include "base/functional/bind.h"
#include "base/logging.h"
#include "gin/arguments.h"
#include "gin/function_template.h"
#include "neva/pal_service/public/mojom/constants.mojom.h"
#include "third_party/blink/public/common/thread_safe_browser_interface_broker_proxy.h"
#include "third_party/blink/public/platform/platform.h"
#include "third_party/blink/public/platform/scheduler/web_agent_group_scheduler.h"
#include "third_party/blink/public/web/blink.h"
#include "third_party/blink/public/web/web_local_frame.h"
#include "third_party/blink/public/web/web_script_source.h"
#include "v8/include/cppgc/allocation.h"

namespace injections {

const char LaunchPointInjection::kLaunchPointObjectName[] = "launchPoint";
const char LaunchPointInjection::kAddLaunchPointMethodName[] = "addLaunchPoint";

// static
void LaunchPointInjection::Install(blink::WebLocalFrame* frame) {
  v8::Isolate* isolate = frame->GetAgentGroupScheduler()->Isolate();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = frame->MainWorldScriptContext();
  if (context.IsEmpty()) {
    return;
  }

  v8::Local<v8::Object> global = context->Global();
  v8::Context::Scope context_scope(context);

  v8::Local<v8::String> launchpoint_name =
      gin::StringToV8(isolate, kLaunchPointObjectName);
  v8::Local<v8::Value> launchpoint_value =
      global->Get(context, launchpoint_name).ToLocalChecked();

  if (!launchpoint_value.IsEmpty() && launchpoint_value->IsObject()) {
    return;
  }

  auto* launchpoint_handle = cppgc::MakeGarbageCollected<LaunchPointInjection>(
      isolate->GetCppHeap()->GetAllocationHandle());

  v8::Local<v8::Object> launchpoint_handle_wrapper;
  if (!launchpoint_handle->GetWrapper(isolate).ToLocal(
          &launchpoint_handle_wrapper)) {
    return;
  }

  global
      ->Set(isolate->GetCurrentContext(), launchpoint_name,
            launchpoint_handle_wrapper)
      .Check();
}

// static
void LaunchPointInjection::Uninstall(blink::WebLocalFrame* frame) {
  v8::Isolate* isolate = frame->GetAgentGroupScheduler()->Isolate();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = frame->MainWorldScriptContext();
  if (context.IsEmpty()) {
    return;
  }

  v8::Local<v8::Object> global = context->Global();
  v8::Context::Scope context_scope(context);

  v8::Local<v8::String> launchpoint_name =
      gin::StringToV8(isolate, kLaunchPointObjectName);
  v8::Local<v8::Value> launchpoint_value =
      global->Get(context, launchpoint_name).ToLocalChecked();

  if (!launchpoint_value.IsEmpty() && launchpoint_value->IsObject()) {
    std::ignore = global->Delete(context, launchpoint_name);
  }
}

LaunchPointInjection::LaunchPointInjection() {
  blink::Platform::Current()->GetBrowserInterfaceBroker()->GetInterface(
      remote_launchpoint_.BindNewPipeAndPassReceiver());
}

LaunchPointInjection::~LaunchPointInjection() = default;

void LaunchPointInjection::AddLaunchPoint(gin::Arguments* args) {
  std::string launch_point_info;
  if (!args->GetNext(&launch_point_info)) {
    LOG(ERROR) << __func__ << ", wrong 'launch_point_info' argument";
    return;
  }

  v8::Local<v8::Function> callback;
  if (!args->GetNext(&callback)) {
    LOG(ERROR) << __func__ << "(), wrong 'callback' argument";
    return;
  }

  remote_launchpoint_->AddLaunchPoint(
      launch_point_info,
      base::BindOnce(&LaunchPointInjection::OnAddLaunchPointRespond,
                     GetAsWeak(),
                     v8::Global<v8::Function>(args->isolate(), callback)));
}

void LaunchPointInjection::Trace(cppgc::Visitor* visitor) const {
  visitor->Trace(weak_factory_);
  gin::Wrappable<LaunchPointInjection>::Trace(visitor);
}

gin::ObjectTemplateBuilder LaunchPointInjection::GetObjectTemplateBuilder(
    v8::Isolate* isolate) {
  return gin::Wrappable<LaunchPointInjection>::GetObjectTemplateBuilder(isolate)
      .SetMethod(kAddLaunchPointMethodName,
                 &LaunchPointInjection::AddLaunchPoint);
}

void LaunchPointInjection::OnAddLaunchPointRespond(
    v8::Global<v8::Function> callback,
    bool return_value,
    const std::string& launch_point_id,
    int error_code,
    const std::string& error_text) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Object> wrapper;
  if (!GetWrapper(isolate).ToLocal(&wrapper)) {
    LOG(ERROR) << __func__ << "(): can not get wrapper";
    return;
  }

  v8::Local<v8::Context> context;
  if (!wrapper->GetCreationContext().ToLocal(&context)) {
    LOG(ERROR) << __func__ << "(): can not get context";
    return;
  }
  v8::Context::Scope context_scope(context);
  const int argc = 4;
  v8::Local<v8::Value> argv[] = {gin::ConvertToV8(isolate, return_value),
                                 gin::StringToV8(isolate, launch_point_id),
                                 gin::ConvertToV8(isolate, error_code),
                                 gin::StringToV8(isolate, error_text)};
  std::ignore = callback.Get(isolate)->Call(context, wrapper, argc, argv);
}

const gin::WrapperInfo* LaunchPointInjection::wrapper_info() const {
  return &kWrapperInfo;
}

}  // namespace injections
