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

#include "neva/injection/renderer/chrome_extensions/chrome_extensions_injection.h"

#include "base/logging.h"
#include "gin/dictionary.h"
#include "gin/handle.h"
#include "third_party/blink/public/common/thread_safe_browser_interface_broker_proxy.h"
#include "third_party/blink/public/platform/platform.h"
#include "third_party/blink/public/platform/scheduler/web_agent_group_scheduler.h"
#include "third_party/blink/public/web/blink.h"
#include "third_party/blink/public/web/web_local_frame.h"

namespace {
// methods
const char kAddEventListenerMethodName[] = "addEventListener";
const char kGetExtensionsInfoMethodName[] = "getExtensionsInfo";
const char kSelectExtensionMethodName[] = "selectExtension";
const char kOnExtensionTabCreatedMethodName[] = "extensionTabCreated";
const char kOnExtensionTabClosedMethodName[] = "extensionTabClosed";
const char kOnExtensionTabActivatedMethodName[] = "extensionTabActivated";
const char kOnExtensionTabUpdatedMethodName[] = "extensionTabUpdated";
const char kOnExtensionPopupViewCreatedMethodName[] =
    "extensionPopupViewCreated";

// events
const char kOnExtensionTabCreationRequested[] = "create-extension-tab";
const char kOnExtensionTabCloseRequested[] = "close-extension-tab";
const char kOnExtensionTabFocusRequested[] = "focus-extension-tab";
const char kOnExtensionPopupCreationRequested[] = "create-extension-popup";
const char kOnExtensionPopupCloseRequested[] = "close-extension-popup";
const char kOnExtensionPopupUpdateRequested[] = "update-extension-popup";
}  // namespace

namespace injections {

gin::WrapperInfo ChromeExtensionsInjection::kWrapperInfo = {
    gin::kEmbedderNativeGin};

ChromeExtensionsInjection::ChromeExtensionsInjection(
    v8::Isolate* isolate,
    mojo::Remote<neva::mojom::NevaExtensionsService> remote)
    : remote_(std::move(remote)), receiver_(this) {
  remote_->BindClient(receiver_.BindNewPipeAndPassRemote());
}

ChromeExtensionsInjection::~ChromeExtensionsInjection() = default;

void ChromeExtensionsInjection::RunAddEventListener(gin::Arguments* args) {
  VLOG(1) << __func__;

  InjectionEventsEmitter::AddEventListener(args);

  if (HasEventListener(kOnExtensionTabCreationRequested)) {
    for (auto& request : pending_tab_creation_requests_) {
      DoEmit(kOnExtensionTabCreationRequested, request.first, request.second);
    }
    pending_tab_creation_requests_.clear();
  }
}

void ChromeExtensionsInjection::GetExtensionsInfo(gin::Arguments* args) {
  v8::Local<v8::Function> local_func;
  if (!args->GetNext(&local_func)) {
    LOG(ERROR) << __func__ << " called without callback function. Do nothing.";
    return;
  }

  auto callback_ptr = std::make_unique<v8::Persistent<v8::Function>>(
      args->isolate(), local_func);

  remote_->GetExtensionsInfo(
      base::BindOnce(&ChromeExtensionsInjection::OnExtensionsInfo,
                     base::Unretained(this), std::move(callback_ptr)));
}

void ChromeExtensionsInjection::OnExtensionsInfo(
    std::unique_ptr<v8::Persistent<v8::Function>> callback,
    std::vector<base::Value> infos) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Object> wrapper = GetWrapper(isolate).ToLocalChecked();
  v8::Local<v8::Context> context;
  if (!wrapper->GetCreationContext().ToLocal(&context))
    return;
  v8::Context::Scope context_scope(context);

  v8::Local<v8::Array> extensions_info_arr =
      v8::Array::New(isolate, infos.size());

  for (size_t i = 0; i < infos.size(); ++i) {
    v8::Local<v8::Object> extension_info_object = v8::Object::New(isolate);
    gin::Dictionary extension_info_dict(isolate, extension_info_object);

    const base::Value::Dict* dict = infos[i].GetIfDict();
    if (dict) {
      std::string id = *dict->FindString("id");
      std::string name = *dict->FindString("name");

      extension_info_dict.Set("id", id);
      extension_info_dict.Set("name", name);
      extensions_info_arr->Set(context, i, extension_info_object).Check();
    }
  }

  v8::Local<v8::Function> local_callback = callback->Get(isolate);
  const int argc = 1;
  v8::Local<v8::Value> argv[] = {extensions_info_arr};
  std::ignore = local_callback->Call(context, wrapper, argc, argv);
}

void ChromeExtensionsInjection::SelectExtension(
    uint64_t tab_id,
    const std::string& extension_id) {
  remote_->OnExtensionSelected(tab_id, extension_id);
}

void ChromeExtensionsInjection::OnExtensionTabCreationRequested(
    uint64_t request_id,
    const std::string& tab_type) {
  VLOG(1) << __func__;

  if (HasEventListener(kOnExtensionTabCreationRequested))
    DoEmit(kOnExtensionTabCreationRequested, request_id, tab_type);
  else
    pending_tab_creation_requests_.emplace_back(request_id, tab_type);
}

void ChromeExtensionsInjection::OnExtensionTabCloseRequested(uint64_t tab_id) {
  VLOG(1) << __func__;
  DoEmit(kOnExtensionTabCloseRequested, tab_id);
}

void ChromeExtensionsInjection::OnExtensionTabFocusRequested(uint64_t tab_id) {
  VLOG(1) << __func__;
  DoEmit(kOnExtensionTabFocusRequested, tab_id);
}

void ChromeExtensionsInjection::OnExtensionTabCreated(gin::Arguments* args) {
  uint64_t request_id = 0, tab_id = 0;
  if (args->Length() > 1) {
    if (!args->GetNext(&request_id) || !args->GetNext(&tab_id)) {
      args->ThrowError();
      return;
    }
    remote_->OnExtensionTabCreatedWithRequestId(request_id, tab_id);
  } else {
    if (!args->GetNext(&tab_id)) {
      args->ThrowError();
      return;
    }
    remote_->OnExtensionTabCreated(tab_id);
  }
}

void ChromeExtensionsInjection::OnExtensionTabClosed(uint64_t tab_id) {
  remote_->OnExtensionTabClosed(tab_id);
}

void ChromeExtensionsInjection::OnExtensionTabActivated(uint64_t tab_id) {
  remote_->OnExtensionTabActivated(tab_id);
}

void ChromeExtensionsInjection::OnExtensionTabUpdated(
    uint64_t tab_id,
    const std::string& change_info) {
  remote_->OnExtensionTabUpdated(tab_id, change_info);
}

void ChromeExtensionsInjection::OnExtensionPopupViewCreated(
    uint64_t popup_view_id,
    uint64_t tab_id) {
  remote_->OnExtensionPopupViewCreated(popup_view_id, tab_id);
}

void ChromeExtensionsInjection::OnExtensionPopupCreationRequested() {
  VLOG(1) << __func__;
  DoEmit(kOnExtensionPopupCreationRequested);
}

void ChromeExtensionsInjection::OnExtensionPopupCloseRequested(
    uint64_t popup_view_id) {
  VLOG(1) << __func__;
  DoEmit(kOnExtensionPopupCloseRequested, popup_view_id);
}

void ChromeExtensionsInjection::OnExtensionPopupUpdateRequested(
    uint64_t popup_view_id,
    int width,
    int height) {
  VLOG(1) << __func__;
  DoEmit(kOnExtensionPopupUpdateRequested, popup_view_id, width, height);
}

// static
gin::ObjectTemplateBuilder ChromeExtensionsInjection::GetObjectTemplateBuilder(
    v8::Isolate* isolate) {
  return gin::Wrappable<ChromeExtensionsInjection>::GetObjectTemplateBuilder(
             isolate)
      .SetMethod(kAddEventListenerMethodName,
                 &ChromeExtensionsInjection::RunAddEventListener)
      .SetMethod(kSelectExtensionMethodName,
                 &ChromeExtensionsInjection::SelectExtension)
      .SetMethod(kGetExtensionsInfoMethodName,
                 &ChromeExtensionsInjection::GetExtensionsInfo)
      .SetMethod(kOnExtensionTabCreatedMethodName,
                 &ChromeExtensionsInjection::OnExtensionTabCreated)
      .SetMethod(kOnExtensionTabClosedMethodName,
                 &ChromeExtensionsInjection::OnExtensionTabClosed)
      .SetMethod(kOnExtensionTabActivatedMethodName,
                 &ChromeExtensionsInjection::OnExtensionTabActivated)
      .SetMethod(kOnExtensionTabUpdatedMethodName,
                 &ChromeExtensionsInjection::OnExtensionTabUpdated)
      .SetMethod(kOnExtensionPopupViewCreatedMethodName,
                 &ChromeExtensionsInjection::OnExtensionPopupViewCreated);
}

}  // namespace injections
