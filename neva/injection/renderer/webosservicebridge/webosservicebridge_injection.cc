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

#include "neva/injection/renderer/webosservicebridge/webosservicebridge_injection.h"

#include <set>
#include <string>

#include "base/functional/bind.h"
#include "base/json/json_reader.h"
#include "base/logging.h"
#include "base/memory/scoped_refptr.h"
#include "base/rand_util.h"
#include "base/values.h"
#include "gin/dictionary.h"
#include "gin/function_template.h"
#include "neva/injection/renderer/gin/function_template_neva.h"
#include "neva/injection/renderer/grit/injection_resources.h"
#include "neva/injection/renderer/webosservicebridge/webosservicebridge_properties.h"
#include "neva/pal_service/public/mojom/constants.mojom.h"
#include "third_party/blink/public/common/thread_safe_browser_interface_broker_proxy.h"
#include "third_party/blink/public/platform/platform.h"
#include "third_party/blink/public/platform/scheduler/web_agent_group_scheduler.h"
#include "third_party/blink/public/web/blink.h"
#include "third_party/blink/public/web/web_local_frame.h"
#include "third_party/blink/public/web/web_script_source.h"
#include "ui/base/resource/resource_bundle.h"
#include "v8/include/cppgc/allocation.h"

namespace {

const char kCallMethodName[] = "call";
const char kCancelMethodName[] = "cancel";
const char kOnServiceCallbackName[] = "onservicecallback";
const char kWebOSServiceBridge[] = "WebOSServiceBridge";
const char kWebOSSystemGetIdentifierJS[] = "webOSSystem.getIdentifier()";
const char kWebOSSystemOnCloseNotifyJS[] =
    "webOSSystem.onCloseNotify(\"didRunOnCloseCallback\")";
const char kMethodInvocationAsConstructorOnly[] =
    "WebOSServiceBridge function must be invoked as a constructor only";

bool IsSubscription(const std::string& param) {
  std::optional<base::Value> json =
      base::JSONReader::Read(param, base::JSON_PARSE_CHROMIUM_EXTENSIONS);
  if (!json || !json->is_dict())
    return false;

  const base::DictValue& dict = json->GetDict();
  if (dict.FindBool("subscribe").value_or(false)) {
    return true;
  }

  return dict.FindBool("watch").value_or(false);
}

}  // anonymous namespace

namespace injections {

bool WebOSServiceBridgeInjection::is_closing_ = false;
std::set<WebOSServiceBridgeInjection*>
    WebOSServiceBridgeInjection::waiting_responses_;

WebOSServiceBridgeInjection::WebOSServiceBridgeInjection(std::string appid)
    : identifier_(std::move(appid))
    , client_receiver_(this) {
  mojo::Remote<pal::mojom::SystemServiceBridgeProvider> provider;
  blink::Platform::Current()->GetBrowserInterfaceBroker()->GetInterface(
      provider.BindNewPipeAndPassReceiver());

  provider->GetSystemServiceBridge(
      remote_system_bridge_.BindNewPipeAndPassReceiver());

  auto params = pal::mojom::ConnectionParams::New(
      std::make_optional<std::string>(),
      std::make_optional<std::string>(identifier_), -1);

  remote_system_bridge_->Connect(
      std::move(params),
      base::BindRepeating(&WebOSServiceBridgeInjection::OnConnect,
                          GetAsWeak()));
}

WebOSServiceBridgeInjection::~WebOSServiceBridgeInjection() {}

void WebOSServiceBridgeInjection::Call(gin::Arguments* args) {
  std::string uri;
  std::string payload;
  if (args->GetNext(&uri) && args->GetNext(&payload))
    DoCall(std::move(uri), std::move(payload));
  else
    DoCall(std::string(""), std::string(""));
}

void WebOSServiceBridgeInjection::DoCall(std::string uri, std::string payload) {
  if (identifier_.empty())
    return;

  remote_system_bridge_->Call(uri, payload);
  if (WebOSServiceBridgeInjection::is_closing_) {
    VLOG(1) << "WebOSServiceBridge [Call][" << identifier_ << "] uri: " << uri
            << ", payload: " << payload << " while closing";
    waiting_responses_.insert(this);
  }

  subscribed_ = IsSubscription(payload);
  PreserveReferenceToInjectionObject(true);
}

void WebOSServiceBridgeInjection::Cancel() {
  if (identifier_.empty())
    return;

  remote_system_bridge_->Cancel();
  waiting_responses_.erase(this);

  PreserveReferenceToInjectionObject(false);
  subscribed_ = false;
}

void WebOSServiceBridgeInjection::DoNothing() {}

void WebOSServiceBridgeInjection::OnConnect(
      mojo::PendingAssociatedReceiver<pal::mojom::SystemServiceBridgeClient>
          receiver) {
  client_receiver_.Bind(std::move(receiver));
}

void WebOSServiceBridgeInjection::CallJSHandler(const std::string& body) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  v8::MaybeLocal<v8::Object> maybe_wrapper = GetWrapper(isolate);
  v8::Local<v8::Object> wrapper;
  if (!maybe_wrapper.ToLocal(&wrapper))
    return;

  v8::Local<v8::Context> context;
  if (!wrapper->GetCreationContext().ToLocal(&context))
    return;
  v8::Context::Scope context_scope(context);
  v8::Local<v8::String> callback_key(
      gin::StringToV8(isolate, kOnServiceCallbackName));

  v8::Local<v8::Value> func_value =
      wrapper->Get(context, callback_key).ToLocalChecked();
  if (func_value->IsNullOrUndefined() || !func_value->IsFunction())
    return;

  v8::Local<v8::Function> func = v8::Local<v8::Function>::Cast(func_value);

  const int argc = 1;
  v8::Local<v8::Value> argv[] = { gin::StringToV8(isolate, body) };
  func->Call(context, wrapper, argc, argv);
}

void WebOSServiceBridgeInjection::Response(pal::mojom::ResponseStatus status,
                                           const std::string& body) {
  if (status == pal::mojom::ResponseStatus::kSuccess)
    CallJSHandler(body);

  // If it was a single response, now it's safe to GC the object
  if (!subscribed_)
    PreserveReferenceToInjectionObject(false);

  if (!WebOSServiceBridgeInjection::is_closing_)
    return;

  VLOG(1) << "WebOSServiceBridge [Response][" << identifier_
          << "] body: " << body << " while closing";

  waiting_responses_.erase(this);

  if (waiting_responses_.empty())
    CloseNotify();
}

gin::ObjectTemplateBuilder
WebOSServiceBridgeInjection::GetObjectTemplateBuilder(v8::Isolate* isolate) {
  return gin::Wrappable<WebOSServiceBridgeInjection>::GetObjectTemplateBuilder(
             isolate)
      .SetMethod(kCallMethodName, &WebOSServiceBridgeInjection::Call)
      .SetMethod(kCancelMethodName, &WebOSServiceBridgeInjection::Cancel)
      .SetMethod(kOnServiceCallbackName,
                 &WebOSServiceBridgeInjection::DoNothing);
}

bool WebOSServiceBridgeInjection::IsWebOSSystemLoaded() {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  v8::MaybeLocal<v8::Object> maybe_wrapper = GetWrapper(isolate);

  v8::Local<v8::Object> wrapper;
  if (!maybe_wrapper.ToLocal(&wrapper))
    return false;

  v8::Local<v8::Context> context;
  if (!wrapper->GetCreationContext().ToLocal(&context))
    return false;
  if (context.IsEmpty())
    return false;

  v8::Local<v8::Object> global = context->Global();
  v8::Maybe<bool> success =
      global->Has(context, gin::StringToV8(isolate, "webOSSystem"));
  return success.IsJust() && success.FromJust();
}

void WebOSServiceBridgeInjection::CloseNotify() {
  if (!IsWebOSSystemLoaded())
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
  v8::Context::Scope context_scope(context);
  v8::Local<v8::String> source =
      gin::StringToV8(isolate, kWebOSSystemOnCloseNotifyJS);
  v8::MaybeLocal<v8::Script> script = v8::Script::Compile(context, source);
  if (!script.IsEmpty())
    script.ToLocalChecked()->Run(context);
}

void WebOSServiceBridgeInjection::PreserveReferenceToInjectionObject(
    bool preserve) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Object> wrapper;
  if (!GetWrapper(isolate).ToLocal(&wrapper))
    return;

  v8::Local<v8::Context> context;
  if (!wrapper->GetCreationContext().ToLocal(&context))
    return;
  if (context.IsEmpty())
    return;

  v8::Local<v8::Object> global = context->Global();
  v8::Context::Scope context_scope(context);

  v8::Local<v8::String> pool_name =
      gin::StringToV8(isolate, "__WebOSServiceBridgePool");
  v8::Local<v8::Value> pool_val;
  if (!global->Get(context, pool_name).ToLocal(&pool_val)) {
    VLOG(1) << "WebOSServiceBridge: requests pool is not available.";
    return;
  }

  if (pool_val->IsSet()) {
    v8::Local<v8::Set> pool_set = v8::Local<v8::Set>::Cast(pool_val);
    if (preserve)
      pool_set->Add(context, wrapper);
    else
      pool_set->Delete(context, wrapper);
  }
}

void WebOSServiceBridgeInjection::Install(blink::WebLocalFrame* frame) {
  v8::Isolate* isolate = frame->GetAgentGroupScheduler()->Isolate();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = frame->MainWorldScriptContext();
  if (context.IsEmpty())
    return;

  v8::Context::Scope context_scope(context);
  v8::Local<v8::Object> global = context->Global();


  scoped_refptr<WebOSServiceBridgeProperties> properties =
      base::MakeRefCounted<WebOSServiceBridgeProperties>(frame);
  if (!properties)
    return;

  v8::Local<v8::FunctionTemplate> templ = gin::CreateConstructorTemplate(
      isolate,
      base::BindRepeating(
          &WebOSServiceBridgeInjection::WebOSServiceBridgeConstructorCallback,
          base::RetainedRef(properties)));
  global
      ->Set(context, gin::StringToSymbol(isolate, kWebOSServiceBridge),
            templ->GetFunction(context).ToLocalChecked())
      .Check();

  const std::string extra_objects_js =
      ui::ResourceBundle::GetSharedInstance().LoadDataResourceString(
          IDR_WEBOSSERVICEBRIDGE_INJECTION_JS);

  v8::Local<v8::Script> local_script;
  v8::MaybeLocal<v8::Script> script = v8::Script::Compile(
      context, gin::StringToV8(isolate, extra_objects_js.c_str()));
  if (script.ToLocal(&local_script))
    local_script->Run(context);
}

// static
void WebOSServiceBridgeInjection::Uninstall(blink::WebLocalFrame* frame) {
  const std::string extra_objects_js =
      ui::ResourceBundle::GetSharedInstance().LoadDataResourceString(
          IDR_WEBOSSERVICEBRIDGE_ROLLBACK_JS);

  v8::Isolate* isolate = frame->GetAgentGroupScheduler()->Isolate();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = frame->MainWorldScriptContext();
  if (context.IsEmpty())
    return;

  v8::Context::Scope context_scope(context);
  v8::Local<v8::Script> local_script;
  v8::MaybeLocal<v8::Script> script = v8::Script::Compile(
      context, gin::StringToV8(isolate, extra_objects_js.c_str()));

  if (script.ToLocal(&local_script))
    local_script->Run(context);
}

// static
bool WebOSServiceBridgeInjection::HasWaitingRequests() {
  return !WebOSServiceBridgeInjection::waiting_responses_.empty();
}

// static
bool WebOSServiceBridgeInjection::IsClosing() {
  return WebOSServiceBridgeInjection::is_closing_;
}

// static
void WebOSServiceBridgeInjection::SetAppInClosing(bool closing) {
  WebOSServiceBridgeInjection::is_closing_ = closing;
}

void WebOSServiceBridgeInjection::Trace(cppgc::Visitor* visitor) const {
  visitor->Trace(weak_factory_);
  gin::Wrappable<WebOSServiceBridgeInjection>::Trace(visitor);
}

// static
void WebOSServiceBridgeInjection::WebOSServiceBridgeConstructorCallback(
    WebOSServiceBridgeProperties* properties,
    gin::Arguments* args) {
  if (!args->IsConstructCall()) {
    args->isolate()->ThrowException(v8::Exception::Error(
        gin::StringToV8(args->isolate(), kMethodInvocationAsConstructorOnly)));
    return;
  }

  if (!properties)
    return;

  std::string appid = properties->GetIdentifier();

  v8::Isolate* isolate = args->isolate();
  v8::HandleScope handle_scope(isolate);

  auto* handle =
      cppgc::MakeGarbageCollected<injections::WebOSServiceBridgeInjection>(
          isolate->GetCppHeap()->GetAllocationHandle(), std::move(appid));

  v8::Local<v8::Object> handle_wrapper;
  if (handle->GetWrapper(isolate).ToLocal(&handle_wrapper)) {
    args->Return(handle_wrapper);
  }
}

const gin::WrapperInfo* WebOSServiceBridgeInjection::wrapper_info() const {
  return &kWrapperInfo;
}

}  // namespace injections
