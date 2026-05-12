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
#include "gin/wrappable.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "neva/browser_shell/service/public/mojom/browser_shell_service.mojom.h"
#include "v8/include/v8.h"

namespace blink {
class WebLocalFrame;
}

namespace injections {

class BrowserShellInjection : public gin::Wrappable<BrowserShellInjection>,
                              public browser_shell::mojom::ShellServiceClient {
 public:
  static gin::WrapperInfo kWrapperInfo;

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

  v8::Local<v8::Object> GetSession(v8::Isolate* isolate,
                                   const std::string& partition);
  v8::Local<v8::Object> GetShellWindow(v8::Isolate* isolate);
  v8::Local<v8::Value> GetLaunchArgs(v8::Isolate* isolate);
  v8::Local<v8::Object> GetPermissions(v8::Isolate* isolate);
  void CreateWindow();

  // browser_shell::mojom::ShellServiceClient
  void SetLaunchParams(const std::string& json) override;
  void Updated() override;

 private:
  gin::ObjectTemplateBuilder GetObjectTemplateBuilder(
      v8::Isolate* isolate) final;

  mojo::Remote<browser_shell::mojom::ShellService> remote_;
  mojo::Receiver<browser_shell::mojom::ShellServiceClient> receiver_;
  std::map<std::string, v8::Global<v8::Object>> sessions_;
  v8::Global<v8::Object> shell_window_object_;
  v8::Global<v8::Value> launch_args_value_;
  v8::Global<v8::Object> permissions_object_;
};

}  // namespace injections

#endif  // NEVA_INJECTION_RENDERER_BROWSER_SHELL_BROWSER_SHELL_INJECTION_H_
