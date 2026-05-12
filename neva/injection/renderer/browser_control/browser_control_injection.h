// Copyright 2018-2019 LG Electronics, Inc.
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

#ifndef NEVA_INJECTION_RENDERER_BROWSER_CONTROL_BROWSER_CONTROL_INJECTION_H_
#define NEVA_INJECTION_RENDERER_BROWSER_CONTROL_BROWSER_CONTROL_INJECTION_H_

#include <string>

#include "gin/object_template_builder.h"
#include "gin/wrappable.h"
#include "neva/injection/renderer/injection_browser_control_base.h"
#include "v8/include/v8.h"

namespace blink {
class WebLocalFrame;
}  // namespace blink

namespace gin {
class Arguments;
}  // namespace gin

namespace injections {

class BrowserControlInjection : public gin::Wrappable<BrowserControlInjection>,
                                public InjectionBrowserControlBase {
 public:
  static gin::WrapperInfo kWrapperInfo;
  static void Install(blink::WebLocalFrame* frame);
  static void Uninstall(blink::WebLocalFrame* frame);

  explicit BrowserControlInjection(blink::WebLocalFrame* frame);
  BrowserControlInjection(const BrowserControlInjection&) = delete;
  BrowserControlInjection& operator=(const BrowserControlInjection&) = delete;
  ~BrowserControlInjection() override;

  void DoCallFunction(gin::Arguments* args);
  void DoSendCommand(gin::Arguments* args);

 private:
  static const char kCallFunctionMethodName[];
  static const char kSendCommandMethodName[];
  static v8::MaybeLocal<v8::Object> CreateObject(blink::WebLocalFrame* frame,
                                                 v8::Isolate* isolate,
                                                 v8::Local<v8::Object> global);

  gin::ObjectTemplateBuilder GetObjectTemplateBuilder(
      v8::Isolate* isolate) final;
};

}  // namespace injections

#endif  // NEVA_INJECTION_RENDERER_BROWSER_CONTROL_BROWSER_CONTROL_INJECTION_H_
