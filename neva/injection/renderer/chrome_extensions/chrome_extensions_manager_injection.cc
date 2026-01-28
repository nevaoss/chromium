// Copyright 2024 LG Electronics, Inc.
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

#include "neva/injection/renderer/chrome_extensions/chrome_extensions_manager_injection.h"

#include "base/notimplemented.h"
#include "gin/dictionary.h"
#include "neva/injection/renderer/chrome_extensions/chrome_extensions_injection.h"
#include "neva/logging.h"
#include "third_party/blink/public/common/thread_safe_browser_interface_broker_proxy.h"
#include "third_party/blink/public/platform/platform.h"
#include "third_party/blink/public/platform/scheduler/web_agent_group_scheduler.h"
#include "third_party/blink/public/web/blink.h"
#include "third_party/blink/public/web/web_local_frame.h"
#include "v8/include/cppgc/allocation.h"

namespace {
// Access via window object
const char kExtensionsManagerObjectName[] = "nevaExtensionsManager";
}  // namespace

namespace injections {

// methods
char ChromeExtensionsManagerInjection::kGetExtensionsServiceForMethodName[] =
    "getExtensionsServiceFor";

// static
void ChromeExtensionsManagerInjection::Install(blink::WebLocalFrame* frame) {
  v8::Isolate* isolate = frame->GetAgentGroupScheduler()->Isolate();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = frame->MainWorldScriptContext();
  if (context.IsEmpty()) {
    return;
  }

  v8::Local<v8::Object> global = context->Global();
  v8::Context::Scope context_scope(context);
  v8::Local<v8::Value> extensions_manager_value =
      global
          ->Get(context, gin::StringToV8(isolate, kExtensionsManagerObjectName))
          .ToLocalChecked();

  if (!extensions_manager_value.IsEmpty() &&
      extensions_manager_value->IsObject()) {
    return;
  }

  auto* extensions_manager =
      cppgc::MakeGarbageCollected<ChromeExtensionsManagerInjection>(
          isolate->GetCppHeap()->GetAllocationHandle(), isolate);

  v8::Local<v8::Object> extensions_manager_object;
  if (!extensions_manager->GetWrapper(isolate).ToLocal(
          &extensions_manager_object)) {
    return;
  }

  global
      ->Set(isolate->GetCurrentContext(),
            gin::StringToV8(isolate, kExtensionsManagerObjectName),
            extensions_manager_object)
      .Check();
}

void ChromeExtensionsManagerInjection::Uninstall(blink::WebLocalFrame* frame) {
  NOTIMPLEMENTED();
}

void ChromeExtensionsManagerInjection::OnExtensionServiceRegistered(
    const std::string& partition) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = v8::Context::New(isolate);
  v8::Context::Scope context_scope(context);

  mojo::Remote<neva::mojom::NevaExtensionsService> remote_extensions_service;
  auto pending_receiver =
      remote_extensions_service.BindNewPipeAndPassReceiver();
  remote_->BindNewExtensionsServiceByContext(std::move(pending_receiver),
                                             partition);
  auto* handle =
      cppgc::MakeGarbageCollected<injections::ChromeExtensionsInjection>(
          isolate->GetCppHeap()->GetAllocationHandle(), isolate,
          std::move(remote_extensions_service));

  services_map_.insert({std::move(partition), handle});
}

void ChromeExtensionsManagerInjection::Trace(cppgc::Visitor* visitor) const {
  for (const auto& service : services_map_) {
    visitor->Trace(service.second);
  }
  InjectionEventsEmitter<ChromeExtensionsManagerInjection>::Trace(visitor);
  gin::Wrappable<ChromeExtensionsManagerInjection>::Trace(visitor);
}

v8::Local<v8::Object> ChromeExtensionsManagerInjection::GetExtensionsServiceFor(
    const std::string& partition) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  auto service = services_map_.find(partition);
  if (service == services_map_.end()) {
    return v8::Local<v8::Object>();
  }
  return service->second->GetWrapper(isolate).ToLocalChecked();
}

ChromeExtensionsManagerInjection::ChromeExtensionsManagerInjection(
    v8::Isolate* isolate)
    : receiver_(this) {
  blink::Platform::Current()->GetBrowserInterfaceBroker()->GetInterface(
      remote_.BindNewPipeAndPassReceiver());

  remote_->RegisterClient(receiver_.BindNewEndpointAndPassRemote());
}

ChromeExtensionsManagerInjection::~ChromeExtensionsManagerInjection() = default;

// static
gin::ObjectTemplateBuilder
ChromeExtensionsManagerInjection::GetObjectTemplateBuilder(
    v8::Isolate* isolate) {
  return gin::Wrappable<ChromeExtensionsManagerInjection>::
      GetObjectTemplateBuilder(isolate)
          .SetMethod(
              ChromeExtensionsManagerInjection::
                  kGetExtensionsServiceForMethodName,
              &ChromeExtensionsManagerInjection::GetExtensionsServiceFor);
}

const gin::WrapperInfo* ChromeExtensionsManagerInjection::wrapper_info() const {
  return &kWrapperInfo;
}

}  // namespace injections
