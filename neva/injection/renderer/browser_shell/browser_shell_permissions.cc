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

#include "neva/injection/renderer/browser_shell/browser_shell_permissions.h"

#include "v8/include/cppgc/allocation.h"
#include "v8/include/v8-cppgc.h"

namespace injections {

namespace events {

const char kPermissionRequest[] = "permission-request";

}  // namespace events

BrowserShellPermissions::BrowserShellPermissions(
    v8::Isolate* isolate,
    mojo::Remote<browser_shell::mojom::Permissions> remote)
    : remote_(std::move(remote)) {
  remote_->RegisterClient(receiver_.BindNewEndpointAndPassRemote());
}

BrowserShellPermissions::~BrowserShellPermissions() = default;

void BrowserShellPermissions::OnPermissionRequested(
    const std::string& host,
    const std::vector<std::string>& types,
    uint64_t request_id,
    uint64_t page_id) {
  if (!GetListenerCount(events::kPermissionRequest)) {
    AckPermission(PermissionRequest::Decision::kIgnore, request_id, page_id);
    return;
  }

  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Object> wrapper = GetWrapper(isolate).ToLocalChecked();
  v8::Local<v8::Context> context;
  if (!wrapper->GetCreationContext().ToLocal(&context))
    return;

  v8::MicrotasksScope microtasksScope(context,
                                      v8::MicrotasksScope::kRunMicrotasks);
  v8::Context::Scope context_scope(context);

  v8::Local<v8::Object> request_permission = v8::Object::New(isolate);

  request_permission
      ->Set(context, gin::StringToV8(isolate, "host"),
            gin::StringToV8(isolate, host))
      .Check();

  request_permission
      ->Set(context, gin::StringToV8(isolate, "types"),
            gin::Converter<std::vector<std::string>>::ToV8(isolate, types))
      .Check();

  request_permission
      ->Set(context, gin::StringToV8(isolate, "page"),
            gin::Converter<uint64_t>::ToV8(isolate, page_id))
      .Check();

  auto* request_obj = cppgc::MakeGarbageCollected<PermissionRequest>(
      isolate->GetCppHeap()->GetAllocationHandle(), this, request_id, page_id);

  v8::Local<v8::Object> request_obj_wrapper;
  if (!request_obj->GetWrapper(isolate).ToLocal(&request_obj_wrapper)) {
    return;
  }

  request_permission
      ->Set(context, gin::StringToV8(isolate, "request"), request_obj_wrapper)
      .Check();

  DoEmit(events::kPermissionRequest, request_permission);
}

void BrowserShellPermissions::AckPermission(PermissionRequest::Decision result,
                                            uint64_t request_id,
                                            uint64_t page_id) {
  if (remote_.is_bound()) {
    browser_shell::mojom::PermissionResult ack =
      browser_shell::mojom::PermissionResult::IGNORED;
    switch(result) {
      case PermissionRequest::Decision::kAllow:
        ack = browser_shell::mojom::PermissionResult::ACCEPTED;
        break;
      case PermissionRequest::Decision::kAllowThisTime:
        ack = browser_shell::mojom::PermissionResult::ACCEPTED_THIS_TIME;
        break;
      case PermissionRequest::Decision::kDeny:
        ack = browser_shell::mojom::PermissionResult::DENIED;
        break;
      case PermissionRequest::Decision::kDismiss:
        ack = browser_shell::mojom::PermissionResult::DISMISSED;
        break;
      case PermissionRequest::Decision::kIgnore:
        ack = browser_shell::mojom::PermissionResult::IGNORED;
        break;
    }
    remote_->DecidePermission(ack, request_id, page_id);
  }
}

void BrowserShellPermissions::Trace(cppgc::Visitor* visitor) const {
  InjectionEventsEmitter<BrowserShellPermissions>::Trace(visitor);
  gin::Wrappable<BrowserShellPermissions>::Trace(visitor);
}

gin::ObjectTemplateBuilder BrowserShellPermissions::GetObjectTemplateBuilder(
    v8::Isolate* isolate) {
  return gin::Wrappable<BrowserShellPermissions>::GetObjectTemplateBuilder(
             isolate)
      .SetMethod(kEmitMethodName, &BrowserShellPermissions::RunEmit)
      .SetMethod(kEventNamesMethodName,
                 &BrowserShellPermissions::RunGetEventNames)
      .SetMethod(kOnMethodName, &BrowserShellPermissions::RunAddEventListener)
      .SetMethod(kOnceMethodName,
                 &BrowserShellPermissions::RunAddOnceEventListener)
      .SetMethod(kListenerCountMethodName,
                 &BrowserShellPermissions::RunGetListenerCount)
      .SetMethod(kRemoveEventListenerMethodName,
                 &BrowserShellPermissions::RunRemoveEventListener)
      .SetMethod(kRemoveAllEventListenersMethodName,
                 &BrowserShellPermissions::RunRemoveAllEventListeners);
}

const gin::WrapperInfo* BrowserShellPermissions::wrapper_info() const {
  return &kWrapperInfo;
}

}  // namespace injections
