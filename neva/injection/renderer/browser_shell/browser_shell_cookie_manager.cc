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

#include "neva/injection/renderer/browser_shell/browser_shell_cookie_manager.h"

#include "base/logging.h"

namespace injections {

namespace {
const char kSetCookieOptionMethodName[] = "setCookieOption";
const char kClearAllCookiesMethodName[] = "clearAllCookies";
const char kGetAllCookiesForTestingMethodName[] = "getAllCookiesForTesting";
}  // namespace

BrowserShellCookieManager::BrowserShellCookieManager(
    v8::Isolate* isolate,
    mojo::Remote<browser_shell::mojom::CookieManager> remote)
    : remote_(std::move(remote)) {
  remote_->BindClient(
      base::BindOnce(&BrowserShellCookieManager::SetupClient, GetAsWeak()));
}

BrowserShellCookieManager::~BrowserShellCookieManager() = default;

void BrowserShellCookieManager::SetupClient(
    mojo::PendingAssociatedReceiver<browser_shell::mojom::CookieManagerClient>
        receiver) {
  client_receiver_.Bind(std::move(receiver));
}

bool BrowserShellCookieManager::SetCookieOption(gin::Arguments* args) {
  int32_t cookie_option;
  bool result = false;
  if (!args->GetNext(&cookie_option)) {
    LOG(ERROR) << __func__ << ", wrong argument";
    return result;
  }

  remote_->SetCookieOption(cookie_option, &result);
  return result;
}

bool BrowserShellCookieManager::ClearAllCookies() {
  bool result = false;
  remote_->ClearAllCookies(&result);
  return result;
}

bool BrowserShellCookieManager::GetAllCookiesForTesting(gin::Arguments* args) {
  v8::Local<v8::Function> callback;
  if (!args->GetNext(&callback)) {
    LOG(ERROR) << __func__ << ", wrong argument";
    return false;
  }

  remote_->GetAllCookiesForTesting(base::BindOnce(
      &BrowserShellCookieManager::OnGetAllCookiesResponse, GetAsWeak(),
      v8::Global<v8::Function>(args->isolate(), callback)));
  return true;
}

void BrowserShellCookieManager::Trace(cppgc::Visitor* visitor) const {
  visitor->Trace(weak_factory_);
  gin::Wrappable<BrowserShellCookieManager>::Trace(visitor);
}

void BrowserShellCookieManager::OnGetAllCookiesResponse(
    v8::Global<v8::Function> callback,
    const std::vector<std::string>& cookie_list) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Object> wrapper = GetWrapper(isolate).ToLocalChecked();
  v8::Local<v8::Context> context;
  if (!wrapper->GetCreationContext().ToLocal(&context)) {
    return;
  }

  v8::Context::Scope context_scope(context);

  v8::Local<v8::Value> result;
  if (gin::TryConvertToV8(isolate, cookie_list, &result)) {
    std::ignore =
        callback.Get(isolate)->Call(context, wrapper, /*argc =*/1, &result);
  }
}

gin::ObjectTemplateBuilder BrowserShellCookieManager::GetObjectTemplateBuilder(
    v8::Isolate* isolate) {
  return gin::Wrappable<BrowserShellCookieManager>::GetObjectTemplateBuilder(
             isolate)
      .SetMethod(kSetCookieOptionMethodName,
                 &BrowserShellCookieManager::SetCookieOption)
      .SetMethod(kClearAllCookiesMethodName,
                 &BrowserShellCookieManager::ClearAllCookies)
      .SetMethod(kGetAllCookiesForTestingMethodName,
                 &BrowserShellCookieManager::GetAllCookiesForTesting);
}

const gin::WrapperInfo* BrowserShellCookieManager::wrapper_info() const {
  return &kWrapperInfo;
}

}  // namespace injections
