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

#ifndef NEVA_INJECTION_RENDERER_BROWSER_SHELL_BROWSER_SHELL_LOGIN_H_
#define NEVA_INJECTION_RENDERER_BROWSER_SHELL_BROWSER_SHELL_LOGIN_H_

#include <string>

#include "base/memory/raw_ptr.h"
#include "gin/object_template_builder.h"
#include "gin/wrappable.h"
#include "neva/browser_shell/service/public/mojom/browser_shell_page_contents.mojom.h"
#include "v8/include/v8.h"

namespace injections {

class BrowserShellLogin : public gin::Wrappable<BrowserShellLogin> {
 public:
  static gin::WrapperInfo kWrapperInfo;

  class Delegate {
   public:
    virtual void AckAuthChallenge(const std::string& login,
                                  const std::string& passwd,
                                  const std::string& url) = 0;
  };

  BrowserShellLogin(Delegate* delegate, browser_shell::mojom::AuthChallengePtr challenge);
  BrowserShellLogin(const BrowserShellLogin&) = delete;
  BrowserShellLogin& operator=(const BrowserShellLogin&) = delete;
  ~BrowserShellLogin() override;

 private:
  // gin::Wrappable.
  gin::ObjectTemplateBuilder GetObjectTemplateBuilder(
      v8::Isolate* isolate) final;

  v8::Local<v8::Object> GetLoginDetails(v8::Isolate* isolate);
  std::string GetURL() const;
  void Response(const std::string& login, const std::string& passwd);

  raw_ptr<Delegate> delegate_;
  std::string url_string_;
  v8::Global<v8::Object> auth_info_;
};

}  // namespace injections

#endif  // NEVA_INJECTION_RENDERER_BROWSER_SHELL_BROWSER_SHELL_LOGIN_H_
