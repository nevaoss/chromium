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

#include "neva/injection/renderer/browser_shell/browser_shell_cookie_manager.h"
#include "neva/injection/renderer/browser_shell/browser_shell_site_settings.h"
#include "neva/injection/renderer/browser_shell/browser_shell_webrequest.h"

namespace injections {

BrowserShellSession::BrowserShellSession(
    v8::Isolate* isolate,
    mojo::Remote<browser_shell::mojom::ShellService>* shell_service,
    std::string partition)
    : shell_service_(shell_service),
      partition_(std::move(partition)) {}

BrowserShellSession::~BrowserShellSession() = default;

void BrowserShellSession::Trace(cppgc::Visitor* visitor) const {
  visitor->Trace(cookie_manager_);
  visitor->Trace(site_settings_);
  visitor->Trace(webrequest_);
  gin::Wrappable<BrowserShellSession>::Trace(visitor);
}

v8::Local<v8::Object> BrowserShellSession::GetCookieManager(
    v8::Isolate* isolate) {
  if (!cookie_manager_) {
    mojo::Remote<browser_shell::mojom::CookieManager> remote_cookie_manager;
    auto pending_receiver = remote_cookie_manager.BindNewPipeAndPassReceiver();
    (*shell_service_)
        ->CreateCookieManager(std::move(pending_receiver), partition_);

    cookie_manager_ =
        cppgc::MakeGarbageCollected<injections::BrowserShellCookieManager>(
            isolate->GetCppHeap()->GetAllocationHandle(), isolate,
            std::move(remote_cookie_manager));
  }

  return cookie_manager_->GetWrapper(isolate).ToLocalChecked();
}

std::string BrowserShellSession::GetPartition() const {
  return partition_;
}

v8::Local<v8::Object> BrowserShellSession::GetSiteSettings(
    v8::Isolate* isolate) {
  if (!site_settings_) {
    mojo::Remote<browser_shell::mojom::SiteSettings> remote_site_settings;
    auto pending_receiver = remote_site_settings.BindNewPipeAndPassReceiver();
    (*shell_service_)
        ->CreateSiteSettings(std::move(pending_receiver), partition_);

    site_settings_ =
        cppgc::MakeGarbageCollected<injections::BrowserShellSiteSettings>(
            isolate->GetCppHeap()->GetAllocationHandle(), isolate,
            std::move(remote_site_settings));
  }

  return site_settings_->GetWrapper(isolate).ToLocalChecked();
}

v8::Local<v8::Object> BrowserShellSession::GetWebRequest(
    v8::Isolate* isolate) {
  if (!webrequest_) {
    mojo::Remote<browser_shell::mojom::WebRequest> remote_webrequest;
    auto pending_receiver = remote_webrequest.BindNewPipeAndPassReceiver();
    (*shell_service_)
        ->CreateWebRequest(std::move(pending_receiver), partition_);

    webrequest_ =
        cppgc::MakeGarbageCollected<injections::BrowserShellWebRequest>(
            isolate->GetCppHeap()->GetAllocationHandle(), isolate,
            std::move(remote_webrequest));
  }

  return webrequest_->GetWrapper(isolate).ToLocalChecked();
}

void BrowserShellSession::Touch() {
  (*shell_service_)->TouchSession(partition_);
}

gin::ObjectTemplateBuilder BrowserShellSession::GetObjectTemplateBuilder(
    v8::Isolate* isolate) {
  return gin::Wrappable<BrowserShellSession>::GetObjectTemplateBuilder(isolate)
      .SetMethod("touch", &BrowserShellSession::Touch)
      .SetProperty("cookiemanager", &BrowserShellSession::GetCookieManager)
      .SetProperty("sitesettings", &BrowserShellSession::GetSiteSettings)
      .SetProperty("webrequest", &BrowserShellSession::GetWebRequest)
      .SetProperty("name", &BrowserShellSession::GetPartition);
}

const gin::WrapperInfo* BrowserShellSession::wrapper_info() const {
  return &kWrapperInfo;
}

}  // namespace injections
