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

#include "neva/injection/renderer/browser_shell/browser_shell_injection.h"

#include "base/functional/bind.h"
#include "base/logging.h"
#include "base/notimplemented.h"
#include "gin/data_object_builder.h"
#include "gin/dictionary.h"
#include "gin/function_template.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "neva/browser_shell/service/public/mojom/browser_shell_window.mojom.h"
#include "neva/injection/renderer/browser_shell/browser_shell_ipc_injection.h"
#include "neva/injection/renderer/browser_shell/browser_shell_page_contents.h"
#include "neva/injection/renderer/browser_shell/browser_shell_page_view.h"
#include "neva/injection/renderer/browser_shell/browser_shell_permissions.h"
#include "neva/injection/renderer/browser_shell/browser_shell_session.h"
#include "neva/injection/renderer/browser_shell/browser_shell_window.h"
#include "neva/injection/renderer/gin/function_template_neva.h"
#include "third_party/blink/public/common/thread_safe_browser_interface_broker_proxy.h"
#include "third_party/blink/public/platform/platform.h"
#include "third_party/blink/public/platform/scheduler/web_agent_group_scheduler.h"
#include "third_party/blink/public/web/blink.h"
#include "third_party/blink/public/web/web_local_frame.h"
#include "v8/include/cppgc/allocation.h"

namespace injections {

const char BrowserShellInjection::kCreateWindowMethodName[] = "createWindow";
const char BrowserShellInjection::kGetSessionMethodName[] = "session";
const char BrowserShellInjection::kLaunchArgsPropertyName[] = "launchArgs";
const char BrowserShellInjection::kPermissionsPropertyName[] = "permissions";
const char BrowserShellInjection::kShellWindowPropertyName[] = "shellWindow";

void BrowserShellInjection::Install(blink::WebLocalFrame* frame) {
  v8::Isolate* isolate = frame->GetAgentGroupScheduler()->Isolate();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = frame->MainWorldScriptContext();
  if (context.IsEmpty())
    return;

  v8::Context::Scope context_scope(context);
  v8::Local<v8::Object> global = context->Global();
  v8::Maybe<bool> has_shell =
      global->Has(context, gin::StringToV8(isolate, "shell"));
  if (has_shell.IsJust() && has_shell.FromJust())
    return;

  auto* shell = cppgc::MakeGarbageCollected<BrowserShellInjection>(
      isolate->GetCppHeap()->GetAllocationHandle(), isolate, global);

  v8::Local<v8::Object> shell_wrapper;
  if (!shell->GetWrapper(isolate).ToLocal(&shell_wrapper)) {
    return;
  }

  global
      ->Set(isolate->GetCurrentContext(), gin::StringToV8(isolate, "shell"),
            shell_wrapper)
      .Check();
}

void BrowserShellInjection::Uninstall(blink::WebLocalFrame* frame) {
  NOTIMPLEMENTED();
}

BrowserShellInjection::BrowserShellInjection(v8::Isolate* isolate,
                                             v8::Local<v8::Object> global) {
  auto context = isolate->GetCurrentContext();

  blink::Platform::Current()->GetBrowserInterfaceBroker()->GetInterface(
      remote_.BindNewPipeAndPassReceiver());

  remote_->RegisterClient(receiver_.BindNewPipeAndPassRemote());

  // Bind Permissions
  mojo::Remote<browser_shell::mojom::Permissions> permissions_remote;
  auto permissions_receiver = permissions_remote.BindNewPipeAndPassReceiver();

  remote_->BindPermissions(std::move(permissions_receiver));

  permissions_ =
      cppgc::MakeGarbageCollected<injections::BrowserShellPermissions>(
          isolate->GetCppHeap()->GetAllocationHandle(), isolate,
          std::move(permissions_remote));

  // PageView Constructor
  v8::Local<v8::FunctionTemplate> page_view_templ =
      gin::CreateConstructorTemplate(
          isolate, base::BindRepeating(
                       &BrowserShellInjection::ConstructPageView, GetAsWeak()));
  global
      ->Set(context, gin::StringToSymbol(isolate, "PageView"),
            page_view_templ->GetFunction(context).ToLocalChecked())
      .Check();

  // PageContents Constructor
  v8::Local<v8::FunctionTemplate> page_contents_templ =
      gin::CreateConstructorTemplate(
          isolate,
          base::BindRepeating(&BrowserShellInjection::ConstructPageContents,
                              GetAsWeak()));
  global
      ->Set(context,
            gin::StringToSymbol(isolate, "PageContents"),
            page_contents_templ->GetFunction(context).ToLocalChecked())
      .Check();

  // ShellIpc constructor
  v8::Local<v8::FunctionTemplate> shell_ipc_templ =
      gin::CreateConstructorTemplate(
          isolate,
          base::BindRepeating(&BrowserShellInjection::ConstructIpcEndpoint,
                              GetAsWeak()));
  global
      ->Set(context,
            gin::StringToSymbol(isolate, "ShellIpc"),
            shell_ipc_templ->GetFunction(context).ToLocalChecked())
      .Check();

  // Bind Shell Window
  mojo::Remote<browser_shell::mojom::ShellWindow> window_remote;
  auto window_receiver = window_remote.BindNewPipeAndPassReceiver();

  shell_window_ = cppgc::MakeGarbageCollected<injections::BrowserShellWindow>(
      isolate->GetCppHeap()->GetAllocationHandle(), isolate,
      std::move(window_remote));

  remote_->BindShellWindow(
      std::move(window_receiver),
      base::BindOnce(&injections::BrowserShellWindow::Setup,
                     shell_window_->GetAsWeak()));
}

BrowserShellInjection::~BrowserShellInjection() = default;

void BrowserShellInjection::ConstructPageContents(gin::Arguments* args) {
  v8::Isolate* isolate = args->isolate();
  v8::HandleScope handle_scope(isolate);

  if (!args->IsConstructCall()) {
    isolate->ThrowException(v8::Exception::Error(
        gin::StringToV8(isolate, "Must be a constructor call")));
    return;
  }

  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  v8::Local<v8::Value> json_value = args->PeekNext();
  v8::Local<v8::Object> json_obj =
      BrowserShellPageContents::BuildOptions(isolate, json_value);
  v8::MaybeLocal<v8::String> maybe_json_str =
      v8::JSON::Stringify(context, json_obj);
  v8::Local<v8::String> json_str;
  std::string json;
  if (maybe_json_str.ToLocal(&json_str)) {
    json = gin::V8ToString(args->isolate(), json_str);
  }

  BrowserShellPageContents::CreateParams params;
  gin::Dictionary json_dict(isolate, json_obj);
  if (!json_dict.Get("error-page-hiding", &params.error_page_hiding)) {
    params.error_page_hiding = false;
  }

  mojo::Remote<browser_shell::mojom::PageContents> remote_contents;
  auto pending_receiver = remote_contents.BindNewPipeAndPassReceiver();

  auto* shell_page_contents =
      cppgc::MakeGarbageCollected<injections::BrowserShellPageContents>(
          isolate->GetCppHeap()->GetAllocationHandle(), isolate,
          std::move(remote_contents), params);

  remote_->CreatePageContents(
      std::move(pending_receiver), json,
      base::BindOnce(&injections::BrowserShellPageContents::Setup,
                     shell_page_contents->GetAsWeak()));

  args->Return(shell_page_contents->GetWrapper(isolate).ToLocalChecked());
}

void BrowserShellInjection::ConstructPageView(gin::Arguments* args) {
  v8::Isolate* isolate = args->isolate();
  v8::HandleScope handle_scope(isolate);

  if (!args->IsConstructCall()) {
    isolate->ThrowException(v8::Exception::Error(
        gin::StringToV8(args->isolate(), "Must be a constructor call")));
    return;
  }

  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  v8::Local<v8::Value> json_value = args->PeekNext();
  v8::Local<v8::Object> json_obj =
      BrowserShellPageView::BuildOptions(isolate, json_value);

  v8::MaybeLocal<v8::String> maybe_json_str =
      v8::JSON::Stringify(context, json_obj);

  v8::Local<v8::String> json_str;
  std::string json;
  if (maybe_json_str.ToLocal(&json_str)) {
    json = gin::V8ToString(args->isolate(), json_str);
  }

  BrowserShellPageView::CreateParams params;
  v8::Local<v8::Object> content_params_obj;
  gin::Dictionary json_dict(isolate, json_obj);
  if (json_dict.Get("page-contents-params", &content_params_obj)) {
    gin::Dictionary content_params_dict(isolate, content_params_obj);
    if (!content_params_dict.Get("error-page-hiding",
                                 &params.error_page_hiding)) {
      params.error_page_hiding = false;
    }
  }

  mojo::Remote<browser_shell::mojom::PageView> remote_view;
  auto pending_receiver = remote_view.BindNewPipeAndPassReceiver();

  auto* shell_page_view =
      cppgc::MakeGarbageCollected<injections::BrowserShellPageView>(
          isolate->GetCppHeap()->GetAllocationHandle(), isolate,
          std::move(remote_view), params);

  remote_->CreatePageView(
      std::move(pending_receiver), json,
      base::BindOnce(&injections::BrowserShellPageView::Setup,
                     shell_page_view->GetAsWeak()));

  args->Return(shell_page_view->GetWrapper(isolate).ToLocalChecked());
}

void BrowserShellInjection::ConstructIpcEndpoint(gin::Arguments* args) {
  v8::Isolate* isolate = args->isolate();
  v8::HandleScope handle_scope(isolate);

  if (!args->IsConstructCall()) {
    isolate->ThrowException(v8::Exception::Error(
        gin::StringToV8(args->isolate(), "Must be a constructor call")));
    return;
  }

  std::string channel;
  if (!args->GetNext(&channel)) {
    return;
  }

  mojo::Remote<browser_shell::mojom::ShellIpcEndpoint> remote_endpoint;
  auto pending_receiver = remote_endpoint.BindNewPipeAndPassReceiver();

  remote_->CreateShellIpcEndpoint(std::move(pending_receiver), channel);
  auto* shell_ipc =
      cppgc::MakeGarbageCollected<injections::BrowserShellIpcEndpoint>(
          isolate->GetCppHeap()->GetAllocationHandle(), isolate, channel,
          std::move(remote_endpoint));

  args->Return(shell_ipc->GetWrapper(isolate).ToLocalChecked());
}

v8::Local<v8::Object> BrowserShellInjection::GetShellWindow(
    v8::Isolate* isolate) {
  return shell_window_->GetWrapper(isolate).ToLocalChecked();
}

v8::Local<v8::Value> BrowserShellInjection::GetLaunchArgs(
    v8::Isolate* isolate) {
  return launch_args_value_.Get(isolate);
}

v8::Local<v8::Object> BrowserShellInjection::GetPermissions(
    v8::Isolate* isolate) {
  return permissions_->GetWrapper(isolate).ToLocalChecked();
}

void BrowserShellInjection::CreateWindow() {
  NOTIMPLEMENTED();
}

v8::Local<v8::Object> BrowserShellInjection::GetSession(
    v8::Isolate* isolate,
    const std::string& partition) {
  auto it = sessions_.find(partition);
  if (it != sessions_.end()) {
    return it->second->GetWrapper(isolate).ToLocalChecked();
  }

  auto* session = cppgc::MakeGarbageCollected<injections::BrowserShellSession>(
      isolate->GetCppHeap()->GetAllocationHandle(), isolate, &remote_,
      partition);
  sessions_[partition] = session;
  return session->GetWrapper(isolate).ToLocalChecked();
}

void BrowserShellInjection::SetLaunchParams(const std::string& json) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  v8::MaybeLocal<v8::Object> maybe_wrapper = GetWrapper(isolate);
  v8::Local<v8::Object> wrapper;
  if (maybe_wrapper.ToLocal(&wrapper)) {
    v8::Local<v8::Context> context;
    if (!wrapper->GetCreationContext().ToLocal(&context))
      return;
    v8::MicrotasksScope microtasksScope(context,
                                        v8::MicrotasksScope::kRunMicrotasks);
    v8::MaybeLocal<v8::Value> maybe_parsed =
        v8::JSON::Parse(context, gin::StringToV8(isolate, json));
    v8::Local<v8::Value> parsed;
    if (maybe_parsed.ToLocal(&parsed))
      launch_args_value_.Reset(isolate, parsed);
  }
}

void BrowserShellInjection::Updated() {
  NOTIMPLEMENTED();
}

void BrowserShellInjection::Trace(cppgc::Visitor* visitor) const {
  for (const auto& session : sessions_) {
    visitor->Trace(session.second);
  }
  visitor->Trace(shell_window_);
  visitor->Trace(launch_args_value_);
  visitor->Trace(permissions_);
  visitor->Trace(weak_factory_);
  gin::Wrappable<BrowserShellInjection>::Trace(visitor);
}

// static
gin::ObjectTemplateBuilder BrowserShellInjection::GetObjectTemplateBuilder(
    v8::Isolate* isolate) {
  return gin::Wrappable<BrowserShellInjection>::GetObjectTemplateBuilder(
             isolate)
      .SetMethod(kCreateWindowMethodName, &BrowserShellInjection::CreateWindow)
      .SetMethod(kGetSessionMethodName, &BrowserShellInjection::GetSession)
      .SetProperty(kLaunchArgsPropertyName,
                   &BrowserShellInjection::GetLaunchArgs)
      .SetProperty(kShellWindowPropertyName,
                   &BrowserShellInjection::GetShellWindow)
      .SetProperty(kPermissionsPropertyName,
                   &BrowserShellInjection::GetPermissions);
}

const gin::WrapperInfo* BrowserShellInjection::wrapper_info() const {
  return &kWrapperInfo;
}

}  // namespace injections
