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

#include "neva/injection/renderer/browser_shell/browser_shell_session.h"

#include "gin/handle.h"
#include "neva/injection/renderer/browser_shell/browser_shell_cookie_manager.h"
#include "neva/injection/renderer/browser_shell/browser_shell_webrequest.h"

namespace injections {

gin::WrapperInfo BrowserShellSession::kWrapperInfo = {gin::kEmbedderNativeGin};

BrowserShellSession::BrowserShellSession(
    v8::Isolate* isolate,
    mojo::Remote<browser_shell::mojom::ShellService>* shell_service,
    std::string partition)
    : shell_service_(shell_service),
      partition_(std::move(partition)) {}

BrowserShellSession::~BrowserShellSession() = default;

v8::Local<v8::Object> BrowserShellSession::GetCookieManager(
    v8::Isolate* isolate) {
  if (!cookie_manager_object_.IsEmpty()) {
    return cookie_manager_object_.Get(isolate);
  }

  mojo::Remote<browser_shell::mojom::CookieManager> remote_cookie_manager;
  auto pending_receiver = remote_cookie_manager.BindNewPipeAndPassReceiver();
  (*shell_service_)
      ->CreateCookieManager(std::move(pending_receiver), partition_);
  gin::Handle<injections::BrowserShellCookieManager> handle = gin::CreateHandle(
      isolate,
      new BrowserShellCookieManager(isolate, std::move(remote_cookie_manager)));

  auto local_cookie_manager = handle->GetWrapper(isolate).ToLocalChecked();
  cookie_manager_object_.Reset(isolate, local_cookie_manager);
  return local_cookie_manager;
}

std::string BrowserShellSession::GetPartition() const {
  return partition_;
}

v8::Local<v8::Object> BrowserShellSession::GetWebRequest(
    v8::Isolate* isolate) {
  if (!webrequest_object_.IsEmpty())
    return webrequest_object_.Get(isolate);

  mojo::Remote<browser_shell::mojom::WebRequest> remote_webrequest;
  auto pending_receiver = remote_webrequest.BindNewPipeAndPassReceiver();
  (*shell_service_)->CreateWebRequest(std::move(pending_receiver), partition_);
  gin::Handle<injections::BrowserShellWebRequest> handle =
      gin::CreateHandle(isolate,
                        new BrowserShellWebRequest(
                            isolate, std::move(remote_webrequest)));

  auto local_webrequest = handle->GetWrapper(isolate).ToLocalChecked();
  webrequest_object_.Reset(isolate, local_webrequest);
  return local_webrequest;
}

void BrowserShellSession::Touch() {
  (*shell_service_)->TouchSession(partition_);
}

gin::ObjectTemplateBuilder BrowserShellSession::GetObjectTemplateBuilder(
    v8::Isolate* isolate) {
  return gin::Wrappable<BrowserShellSession>::GetObjectTemplateBuilder(isolate)
      .SetMethod("touch", &BrowserShellSession::Touch)
      .SetProperty("cookiemanager", &BrowserShellSession::GetCookieManager)
      .SetProperty("webrequest", &BrowserShellSession::GetWebRequest)
      .SetProperty("name", &BrowserShellSession::GetPartition);
}

}  // namespace injections
