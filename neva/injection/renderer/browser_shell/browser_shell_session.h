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

#ifndef NEVA_INJECTION_RENDERER_BROWSER_SHELL_BROWSER_SHELL_SESSION_H_
#define NEVA_INJECTION_RENDERER_BROWSER_SHELL_BROWSER_SHELL_SESSION_H_

#include <string>

#include "base/memory/raw_ptr.h"
#include "gin/object_template_builder.h"
#include "gin/wrappable.h"
#include "mojo/public/cpp/bindings/associated_receiver.h"
#include "mojo/public/cpp/bindings/pending_associated_receiver.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "neva/browser_shell/service/public/mojom/browser_shell_service.mojom.h"
#include "v8/include/v8-cppgc.h"
#include "v8/include/v8.h"

namespace injections {

class BrowserShellCookieManager;
class BrowserShellSiteSettings;
class BrowserShellWebRequest;

class BrowserShellSession final : public gin::Wrappable<BrowserShellSession> {
 public:
  static constexpr gin::WrapperInfo kWrapperInfo = {{gin::kEmbedderNativeGin},
                                                    gin::kBrowserShellSession};

  BrowserShellSession(
      v8::Isolate* isolate,
      mojo::Remote<browser_shell::mojom::ShellService>* shell_service,
      std::string partition);
  BrowserShellSession(const BrowserShellSession&) = delete;
  BrowserShellSession& operator=(const BrowserShellSession&) = delete;
  ~BrowserShellSession() override;

  // gin::Wrappable : v8::Object::Wrappable
  void Trace(cppgc::Visitor* visitor) const final;

 private:
  // gin::WrappableBase
  const gin::WrapperInfo* wrapper_info() const override;

  // gin::Wrappable.
  gin::ObjectTemplateBuilder GetObjectTemplateBuilder(
      v8::Isolate* isolate) final;

  v8::Local<v8::Object> GetCookieManager(v8::Isolate* isolate);
  std::string GetPartition() const;
  v8::Local<v8::Object> GetSiteSettings(v8::Isolate* isolate);
  v8::Local<v8::Object> GetWebRequest(v8::Isolate* isolate);
  void Touch();

  raw_ptr<mojo::Remote<browser_shell::mojom::ShellService>> shell_service_;
  std::string partition_;
  cppgc::Member<BrowserShellCookieManager> cookie_manager_;
  cppgc::Member<BrowserShellSiteSettings> site_settings_;
  cppgc::Member<BrowserShellWebRequest> webrequest_;
};

}  // namespace injections

#endif  // NEVA_INJECTION_RENDERER_BROWSER_SHELL_BROWSER_SHELL_SESSION_H_
