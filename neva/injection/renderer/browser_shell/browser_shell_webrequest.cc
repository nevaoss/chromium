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

#include "neva/injection/renderer/browser_shell/browser_shell_webrequest.h"

#include "gin/arguments.h"
#include "gin/converter.h"
#include "gin/dictionary.h"
#include "gin/function_template.h"
#include "gin/handle.h"

namespace injections {

namespace {

v8::Local<v8::Value> BuildDetailsValue(
    v8::Isolate* isolate,
    const browser_shell::mojom::DetailsPtr& details) {
  v8::Local<v8::Object> details_obj = v8::Object::New(isolate);
  gin::Dictionary dict(isolate, details_obj);
  dict.Set("id", details->id);
  dict.Set("url", details->url);
  dict.Set("method", details->method);
  dict.Set("timestamp", details->timestamp);
  dict.Set("resourceType", details->resource_type);
  if (details->response_ip.has_value())
    dict.Set("ip", *details->response_ip);
  dict.Set("fromCache", details->from_cache);
  return details_obj.As<v8::Value>();
}

void GetBeforeRequestCallbackResult(
    v8::Isolate* isolate,
    v8::Local<v8::Object> result_obj,
    bool* cancel,
    std::string* redirect_url) {
  gin::Dictionary dict(isolate, result_obj);
  dict.Get("cancel", cancel);
  dict.Get("redirectURL", redirect_url);
}

}  // namespace

gin::WrapperInfo BrowserShellWebRequest::kWrapperInfo = {
    gin::kEmbedderNativeGin};

BrowserShellWebRequest::BrowserShellWebRequest(
    v8::Isolate* isolate,
    mojo::Remote<browser_shell::mojom::WebRequest> remote)
    : remote_(std::move(remote)),
      client_receiver_(this) {
  remote_->BindClient(base::BindOnce(&BrowserShellWebRequest::SetupClient,
                                     base::Unretained(this)));
}

BrowserShellWebRequest::~BrowserShellWebRequest() = default;

void BrowserShellWebRequest::SetupClient(
    mojo::PendingAssociatedReceiver<browser_shell::mojom::WebRequestClient>
        receiver) {
  client_receiver_.Bind(std::move(receiver));
}

void BrowserShellWebRequest::SetOnBeforeRequestListener(gin::Arguments* args) {
  v8::Local<v8::Value> arg;

  // urls
  std::string json;
  if (args->GetNext(&arg) && !arg->IsFunction()) {
    v8::Local<v8::String> json_v8;
    auto context = args->isolate()->GetCurrentContext();
    if (v8::JSON::Stringify(context, arg).ToLocal(&json_v8)) {
      json = gin::V8ToString(args->isolate(), json_v8);
    args->GetNext(&arg);
  }

  // listener
  if (!arg.IsEmpty()) {
    if (arg->IsNull()) {
      on_before_request_listener_.Reset();
      remote_->ResetOnBeforeRequestListener();
      return;
    }

    v8::Local<v8::Function> listener;
    if (gin::ConvertFromV8(args->isolate(), arg, &listener)) {
        on_before_request_listener_.Reset(args->isolate(), listener);
        remote_->SetOnBeforeRequestListener(std::move(json));
        return;
      }
    }
  }

  args->ThrowTypeError("Must pass null or a Function");
}

void BrowserShellWebRequest::OnBeforeRequest(
    browser_shell::mojom::DetailsPtr details,
    OnBeforeRequestCallback callback) {
  bool cancel = true;
  std::string redirect_url;
  HandleOnBeforeRequest(details, &cancel, &redirect_url);
  std::move(callback).Run(cancel, std::move(redirect_url));
}

void BrowserShellWebRequest::HandleOnBeforeRequest(
    browser_shell::mojom::DetailsPtr& details,
    bool* cancel,
    std::string* redirect_url) {
  *cancel = true;
  *redirect_url = "";

  if (on_before_request_listener_.IsEmpty())
    return;

  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  v8::MaybeLocal<v8::Object> maybe_wrapper = GetWrapper(isolate);
  v8::Local<v8::Object> wrapper;
  if (!maybe_wrapper.ToLocal(&wrapper))
    return;

  v8::Local<v8::Context> context;
  if (!wrapper->GetCreationContext().ToLocal(&context))
    return;

  v8::MicrotasksScope microtasksScope(context,
                                      v8::MicrotasksScope::kRunMicrotasks);
  v8::Context::Scope context_scope(context);
  const int argc = 1;
  v8::Local<v8::Value> argv[] = { BuildDetailsValue(isolate, details) };
  v8::Local<v8::Function> func = on_before_request_listener_.Get(isolate);

  v8::MaybeLocal<v8::Value> maybe_result =
      func->Call(context, wrapper, argc, argv);

  v8::Local<v8::Value> result;
  if (maybe_result.ToLocal(&result)) {
    GetBeforeRequestCallbackResult(
        isolate, result.As<v8::Object>(), cancel, redirect_url);
  }
}

gin::ObjectTemplateBuilder BrowserShellWebRequest::GetObjectTemplateBuilder(
    v8::Isolate* isolate) {
  return gin::Wrappable<BrowserShellWebRequest>::
      GetObjectTemplateBuilder(isolate)
      .SetMethod("onBeforeRequest",
                 &BrowserShellWebRequest::SetOnBeforeRequestListener);
}

}  // namespace injections
