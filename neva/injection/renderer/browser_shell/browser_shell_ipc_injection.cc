// Copyright 2022 LG Electronics, Inc.
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

#include "neva/injection/renderer/browser_shell/browser_shell_ipc_injection.h"

#include "base/functional/bind.h"
#include "base/json/json_reader.h"
#include "base/logging.h"
#include "base/notimplemented.h"
#include "gin/function_template.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "neva/injection/renderer/gin/function_template_neva.h"
#include "third_party/blink/public/common/thread_safe_browser_interface_broker_proxy.h"
#include "third_party/blink/public/platform/platform.h"
#include "third_party/blink/public/platform/scheduler/web_agent_group_scheduler.h"
#include "third_party/blink/public/web/blink.h"
#include "third_party/blink/public/web/web_local_frame.h"
#include "v8/include/cppgc/allocation.h"
#include "v8/include/v8-cppgc.h"

namespace injections {

const char BrowserShellIpcEndpoint::kChannelPropertyName[] = "channel";
const char BrowserShellIpcEndpoint::kPostMethodName[] = "post";

void BrowserShellIpcInjection::Install(blink::WebLocalFrame* frame) {
  v8::Isolate* isolate = frame->GetAgentGroupScheduler()->Isolate();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = frame->MainWorldScriptContext();
  if (context.IsEmpty())
    return;

  v8::Context::Scope context_scope(context);
  v8::Local<v8::Object> global = context->Global();
  v8::Maybe<bool> has_shell =
      global->Has(context, gin::StringToV8(isolate, "shell"));
  if (has_shell.IsJust() && has_shell.FromJust()) {
    return;
  }

  auto* shell_ipc = cppgc::MakeGarbageCollected<BrowserShellIpcInjection>(
      isolate->GetCppHeap()->GetAllocationHandle(), isolate, global);

  v8::Local<v8::Object> shell_ipc_wrapper;
  if (!shell_ipc->GetWrapper(isolate).ToLocal(&shell_ipc_wrapper)) {
    return;
  }

  global
      ->Set(isolate->GetCurrentContext(), gin::StringToV8(isolate, "shell"),
            shell_ipc_wrapper)
      .Check();
}

void BrowserShellIpcInjection::Uninstall(blink::WebLocalFrame* frame) {
  NOTIMPLEMENTED();
}

BrowserShellIpcInjection::BrowserShellIpcInjection(
    v8::Isolate* isolate,
    v8::Local<v8::Object> global) {
  auto context = isolate->GetCurrentContext();
  blink::Platform::Current()->GetBrowserInterfaceBroker()->GetInterface(
      remote_.BindNewPipeAndPassReceiver());

  // ShellIpc constructor
  v8::Local<v8::FunctionTemplate> shell_ipc_templ =
      gin::CreateConstructorTemplate(
          isolate,
          base::BindRepeating(&BrowserShellIpcInjection::ConstructIpcEndpoint,
                              GetAsWeak()));
  global
      ->Set(context,
            gin::StringToSymbol(isolate, "ShellIpc"),
            shell_ipc_templ->GetFunction(context).ToLocalChecked())
      .Check();
}

BrowserShellIpcInjection::~BrowserShellIpcInjection() = default;

void BrowserShellIpcInjection::ConstructIpcEndpoint(gin::Arguments* args) {
  v8::Isolate* isolate = args->isolate();
  v8::HandleScope handle_scope(isolate);

  if (!args->IsConstructCall()) {
    isolate->ThrowException(v8::Exception::Error(
        gin::StringToV8(args->isolate(), "Must be a constructor call")));
    return;
  }

  std::string channel;
  if (!args->GetNext(&channel))
    return;

  mojo::Remote<browser_shell::mojom::ShellIpcEndpoint> remote_endpoint;
  auto pending_receiver = remote_endpoint.BindNewPipeAndPassReceiver();

  remote_->CreateShellIpcEndpoint(std::move(pending_receiver), channel);
  auto* shell_ipc =
      cppgc::MakeGarbageCollected<injections::BrowserShellIpcEndpoint>(
          isolate->GetCppHeap()->GetAllocationHandle(), isolate, channel,
          std::move(remote_endpoint));

  args->Return(shell_ipc->GetWrapper(isolate).ToLocalChecked());
}

void BrowserShellIpcInjection::Trace(cppgc::Visitor* visitor) const {
  visitor->Trace(weak_factory_);
  gin::Wrappable<BrowserShellIpcInjection>::Trace(visitor);
}

// static
gin::ObjectTemplateBuilder BrowserShellIpcInjection::GetObjectTemplateBuilder(
    v8::Isolate* isolate) {
  return gin::Wrappable<BrowserShellIpcInjection>::GetObjectTemplateBuilder(
      isolate);
}

const gin::WrapperInfo* BrowserShellIpcInjection::wrapper_info() const {
  return &kWrapperInfo;
}

BrowserShellIpcEndpoint::BrowserShellIpcEndpoint(
    v8::Isolate*,
    std::string channel,
    mojo::Remote<browser_shell::mojom::ShellIpcEndpoint> remote)
    : channel_(std::move(channel)), remote_(std::move(remote)) {
  remote_->BindClient(
      base::BindOnce(&BrowserShellIpcEndpoint::Setup, GetAsWeak()));
}

BrowserShellIpcEndpoint::~BrowserShellIpcEndpoint() = default;

void BrowserShellIpcEndpoint::Setup(
    mojo::PendingAssociatedReceiver<
        browser_shell::mojom::ShellIpcEndpointClient> receiver) {
  client_receiver_.Bind(std::move(receiver));
}

const std::string& BrowserShellIpcEndpoint::GetChannelName() const {
  return channel_;
}

void BrowserShellIpcEndpoint::Post(gin::Arguments* args) {
  std::string event;
  if (!args->GetNext(&event))
    return;

  v8::Isolate* isolate = args->isolate();
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  v8::Local<v8::Value> json_value;
  if (args->GetNext(&json_value)) {
    v8::MaybeLocal<v8::String> maybe_json_str =
        v8::JSON::Stringify(context, json_value);
    v8::Local<v8::String> json_str;
    if (maybe_json_str.ToLocal(&json_str)) {
      remote_->Post(event, gin::V8ToString(args->isolate(), json_str));
    }
  } else {
    remote_->Post(event, "{}");
  }
}

void BrowserShellIpcEndpoint::Handle(const std::string& event,
                                     const std::string& json) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  v8::MaybeLocal<v8::Object> maybe_wrapper = GetWrapper(isolate);
  v8::Local<v8::Object> wrapper;
  if (maybe_wrapper.ToLocal(&wrapper)) {
    v8::Local<v8::Context> context;
    if (wrapper->GetCreationContext().ToLocal(&context)) {
      v8::MicrotasksScope microtasksScope(context,
                                      v8::MicrotasksScope::kRunMicrotasks);
      v8::MaybeLocal<v8::Value> maybe_parsed =
          v8::JSON::Parse(context, gin::StringToV8(isolate, json));
      v8::Local<v8::Value> parsed;
      if (maybe_parsed.ToLocal(&parsed))
        DoEmit(event, parsed);
    }
  }
}

void BrowserShellIpcEndpoint::Trace(cppgc::Visitor* visitor) const {
  visitor->Trace(weak_factory_);
  InjectionEventsEmitter<BrowserShellIpcEndpoint>::Trace(visitor);
  gin::Wrappable<BrowserShellIpcEndpoint>::Trace(visitor);
}

// static
gin::ObjectTemplateBuilder BrowserShellIpcEndpoint::GetObjectTemplateBuilder(
    v8::Isolate* isolate) {
  return gin::Wrappable<BrowserShellIpcEndpoint>::GetObjectTemplateBuilder(
             isolate)
      .SetMethod(kPostMethodName, &BrowserShellIpcEndpoint::Post)
      .SetMethod(kEmitMethodName, &BrowserShellIpcEndpoint::RunEmit)
      .SetMethod(kEventNamesMethodName,
                 &BrowserShellIpcEndpoint::RunGetEventNames)
      .SetMethod(kListenerCountMethodName,
                 &BrowserShellIpcEndpoint::RunGetListenerCount)
      .SetMethod(kOnMethodName, &BrowserShellIpcEndpoint::RunAddEventListener)
      .SetMethod(kOnceMethodName,
                 &BrowserShellIpcEndpoint::RunAddOnceEventListener)
      .SetMethod(kRemoveEventListenerMethodName,
                 &BrowserShellIpcEndpoint::RunRemoveEventListener)
      .SetMethod(kRemoveAllEventListenersMethodName,
                 &BrowserShellIpcEndpoint::RunRemoveAllEventListeners)
      .SetProperty(kChannelPropertyName,
                   &BrowserShellIpcEndpoint::GetChannelName);
}

const gin::WrapperInfo* BrowserShellIpcEndpoint::wrapper_info() const {
  return &kWrapperInfo;
}

}  // namespace injections
