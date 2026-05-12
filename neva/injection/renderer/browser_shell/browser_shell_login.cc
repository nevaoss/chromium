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

#include "neva/injection/renderer/browser_shell/browser_shell_login.h"

#include "neva/browser_shell/service/public/mojom/browser_shell_page_contents.mojom.h"

namespace injections {

gin::WrapperInfo BrowserShellLogin::kWrapperInfo = {gin::kEmbedderNativeGin};

BrowserShellLogin::BrowserShellLogin(BrowserShellLogin::Delegate* delegate,
             browser_shell::mojom::AuthChallengePtr challenge)
    : delegate_(delegate), url_string_(challenge->url) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Object> wrapper = GetWrapper(isolate).ToLocalChecked();

  v8::Local<v8::Context> context;
  if (!wrapper->GetCreationContext().ToLocal(&context))
    return;

  v8::Local<v8::Object> auth_info_local = v8::Object::New(isolate);
  auth_info_local
      ->Set(context, gin::StringToV8(isolate, "host"),
            gin::ConvertToV8(isolate, challenge->host))
      .Check();
  auth_info_local
      ->Set(context, gin::StringToV8(isolate, "isProxy"),
            gin::ConvertToV8(isolate, challenge->is_proxy))
      .Check();
  auth_info_local
      ->Set(context, gin::StringToV8(isolate, "port"),
            gin::ConvertToV8(isolate, challenge->port))
      .Check();
  auth_info_local
      ->Set(context, gin::StringToV8(isolate, "realm"),
            gin::ConvertToV8(isolate, challenge->realm))
      .Check();
  auth_info_local
      ->Set(context, gin::StringToV8(isolate, "scheme"),
            gin::ConvertToV8(isolate, challenge->scheme))
      .Check();

  auth_info_.Reset(isolate, auth_info_local);
}

BrowserShellLogin::~BrowserShellLogin() = default;

v8::Local<v8::Object> BrowserShellLogin::GetLoginDetails(v8::Isolate* isolate) {
  return auth_info_.Get(isolate);
}

std::string BrowserShellLogin::GetURL() const {
  return url_string_;
}

void BrowserShellLogin::Response(const std::string& login, const std::string& passwd) {
  delegate_->AckAuthChallenge(login, passwd, url_string_);
}

gin::ObjectTemplateBuilder BrowserShellLogin::GetObjectTemplateBuilder(
    v8::Isolate* isolate) {
  return gin::Wrappable<BrowserShellLogin>::GetObjectTemplateBuilder(isolate)
      .SetMethod("response", &BrowserShellLogin::Response)
      .SetProperty("url", &BrowserShellLogin::GetURL)
      .SetProperty("loginDetails", &BrowserShellLogin::GetLoginDetails);
}

}  // namespace injections
