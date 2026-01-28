// Copyright 2021 LG Electronics, Inc.
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

#ifndef NEVA_INJECTION_RENDERER_BROWSER_SHELL_BROWSER_SHELL_INJECTION_H_
#define NEVA_INJECTION_RENDERER_BROWSER_SHELL_BROWSER_SHELL_INJECTION_H_

#include "gin/object_template_builder.h"
#include "gin/persistent.h"
#include "gin/weak_cell.h"
#include "gin/wrappable.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "neva/browser_shell/service/public/mojom/browser_shell_service.mojom.h"
#include "v8/include/v8-cppgc.h"
#include "v8/include/v8.h"

namespace blink {
class WebLocalFrame;
}

namespace injections {

class BrowserShellPermissions;
class BrowserShellSession;
class BrowserShellWindow;

class BrowserShellInjection final
    : public gin::Wrappable<BrowserShellInjection>,
      public browser_shell::mojom::ShellServiceClient {
 public:
  static constexpr gin::WrapperInfo kWrapperInfo = {
      {gin::kEmbedderNativeGin},
      gin::kBrowserShellInjection};

  static const char kCreateWindowMethodName[];
  static const char kGetSessionMethodName[];
  static const char kLaunchArgsPropertyName[];
  static const char kPermissionsPropertyName[];
  static const char kShellWindowPropertyName[];

  static void Install(blink::WebLocalFrame* frame);
  static void Uninstall(blink::WebLocalFrame* frame);

  BrowserShellInjection(v8::Isolate* isolate, v8::Local<v8::Object> global);
  BrowserShellInjection(const BrowserShellInjection&) = delete;
  BrowserShellInjection& operator=(const BrowserShellInjection&) = delete;
  ~BrowserShellInjection() override;

  cppgc::Persistent<gin::WeakCell<BrowserShellInjection>> GetAsWeak() {
    return gin::WrapPersistent(weak_factory_.GetWeakCell(
        v8::Isolate::GetCurrent()->GetCppHeap()->GetAllocationHandle()));
  }

  void ConstructPageContents(gin::Arguments* args);
  void ConstructPageView(gin::Arguments* args);
  void ConstructIpcEndpoint(gin::Arguments* args);

  v8::Local<v8::Object> GetSession(v8::Isolate* isolate,
                                   const std::string& partition);
  v8::Local<v8::Object> GetShellWindow(v8::Isolate* isolate);
  v8::Local<v8::Value> GetLaunchArgs(v8::Isolate* isolate);
  v8::Local<v8::Object> GetPermissions(v8::Isolate* isolate);
  void CreateWindow();

  // browser_shell::mojom::ShellServiceClient
  void SetLaunchParams(const std::string& json) override;
  void Updated() override;

  // gin::Wrappable : v8::Object::Wrappable
  void Trace(cppgc::Visitor* visitor) const final;

 private:
  // gin::WrappableBase
  const gin::WrapperInfo* wrapper_info() const override;

  gin::ObjectTemplateBuilder GetObjectTemplateBuilder(
      v8::Isolate* isolate) final;

  mojo::Remote<browser_shell::mojom::ShellService> remote_;
  mojo::Receiver<browser_shell::mojom::ShellServiceClient> receiver_{this};
  std::map<std::string, cppgc::Member<injections::BrowserShellSession>>
      sessions_;
  cppgc::Member<injections::BrowserShellWindow> shell_window_;
  v8::TracedReference<v8::Value> launch_args_value_;
  cppgc::Member<injections::BrowserShellPermissions> permissions_;
  gin::WeakCellFactory<BrowserShellInjection> weak_factory_{this};
};

}  // namespace injections

#endif  // NEVA_INJECTION_RENDERER_BROWSER_SHELL_BROWSER_SHELL_INJECTION_H_
