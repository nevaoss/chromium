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
#include "gin/function_template.h"
#include "gin/handle.h"
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

namespace injections {

gin::WrapperInfo BrowserShellInjection::kWrapperInfo = {
    gin::kEmbedderNativeGin
};

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

  gin::Handle<BrowserShellInjection> shell =
      gin::CreateHandle(isolate, new BrowserShellInjection(isolate, global));

  if (shell.IsEmpty())
    return;

  global
      ->Set(isolate->GetCurrentContext(), gin::StringToV8(isolate, "shell"),
            shell.ToV8())
      .Check();
}

void BrowserShellInjection::Uninstall(blink::WebLocalFrame* frame) {
  NOTIMPLEMENTED();
}

BrowserShellInjection::BrowserShellInjection(v8::Isolate* isolate,
                                             v8::Local<v8::Object> global)
  : receiver_(this) {
  auto context = isolate->GetCurrentContext();

  blink::Platform::Current()->GetBrowserInterfaceBroker()->GetInterface(
      remote_.BindNewPipeAndPassReceiver());

  remote_->RegisterClient(receiver_.BindNewPipeAndPassRemote());

  // Bind Permissions
  mojo::Remote<browser_shell::mojom::Permissions> permissions_remote;
  auto permissions_receiver = permissions_remote.BindNewPipeAndPassReceiver();
  auto* permissions = new injections::BrowserShellPermissions(
      isolate, std::move(permissions_remote));

  remote_->BindPermissions(std::move(permissions_receiver));
  gin::Handle<injections::BrowserShellPermissions> permissions_handle =
      gin::CreateHandle(isolate, permissions);

  if (!permissions_handle.IsEmpty()) {
    permissions_object_.Reset(
        isolate, permissions_handle->GetWrapper(isolate).ToLocalChecked());
  }

  // PageView Constructor
  v8::Local<v8::FunctionTemplate> page_view_templ =
      gin::CreateConstructorTemplate(
          isolate,
          base::BindRepeating(&BrowserShellPageView::ConstructorCallback,
                              base::Unretained(&remote_)));
  global
      ->Set(context, gin::StringToSymbol(isolate, "PageView"),
            page_view_templ->GetFunction(context).ToLocalChecked())
      .Check();

  // PageContents Constructor
  v8::Local<v8::FunctionTemplate> page_contents_templ =
      gin::CreateConstructorTemplate(
          isolate,
          base::BindRepeating(&BrowserShellPageContents::ConstructorCallback,
                              base::Unretained(&remote_)));
  global
      ->Set(context,
            gin::StringToSymbol(isolate, "PageContents"),
            page_contents_templ->GetFunction(context).ToLocalChecked())
      .Check();

  // ShellIpc constructor
  v8::Local<v8::FunctionTemplate> shell_ipc_templ =
      gin::CreateConstructorTemplate(
          isolate,
          base::BindRepeating(&BrowserShellIpcInjection::ConstructorCallback,
                              base::Unretained(&remote_)));
  global
      ->Set(context,
            gin::StringToSymbol(isolate, "ShellIpc"),
            shell_ipc_templ->GetFunction(context).ToLocalChecked())
      .Check();

  // Bind Shell Window
  mojo::Remote<browser_shell::mojom::ShellWindow> window_remote;
  auto window_receiver = window_remote.BindNewPipeAndPassReceiver();
  auto* shell_window =
      new injections::BrowserShellWindow(isolate, std::move(window_remote));
  remote_->BindShellWindow(std::move(window_receiver),
                           base::BindOnce(&BrowserShellWindow::Setup,
                                          base::Unretained(shell_window)));
  gin::Handle<injections::BrowserShellWindow> window_handle =
      gin::CreateHandle(isolate, shell_window);

  if (!window_handle.IsEmpty()) {
    shell_window_object_.Reset(
        isolate, window_handle->GetWrapper(isolate).ToLocalChecked());
  }
}

BrowserShellInjection::~BrowserShellInjection() = default;

v8::Local<v8::Object> BrowserShellInjection::GetShellWindow(
    v8::Isolate* isolate) {
  return shell_window_object_.Get(isolate);
}

v8::Local<v8::Value> BrowserShellInjection::GetLaunchArgs(
    v8::Isolate* isolate) {
  return launch_args_value_.Get(isolate);
}

v8::Local<v8::Object> BrowserShellInjection::GetPermissions(
    v8::Isolate* isolate) {
  return permissions_object_.Get(isolate);
}

void BrowserShellInjection::CreateWindow() {
  NOTIMPLEMENTED();
}

v8::Local<v8::Object> BrowserShellInjection::GetSession(
    v8::Isolate* isolate,
    const std::string& partition) {

  auto it = sessions_.find(partition);
  if (it != sessions_.end())
    return it->second.Get(isolate);

  gin::Handle<injections::BrowserShellSession> handle =
      gin::CreateHandle(isolate,
                        new BrowserShellSession(isolate, &remote_, partition));

  auto local_session = handle->GetWrapper(isolate).ToLocalChecked();
  sessions_[partition].Reset(isolate, local_session);
  return local_session;
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

}  // namespace injections
