// Copyright 2016-2019 LG Electronics, Inc.
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

#ifndef NEVA_INJECTION_RENDERER_SAMPLE_SAMPLE_INJECTION_H_
#define NEVA_INJECTION_RENDERER_SAMPLE_SAMPLE_INJECTION_H_

#include <string>

#include "gin/object_template_builder.h"
#include "gin/wrappable.h"
#include "mojo/public/cpp/bindings/associated_receiver.h"
#include "mojo/public/cpp/bindings/pending_associated_receiver.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "neva/pal_service/public/mojom/sample.mojom.h"
#include "v8/include/v8.h"

namespace blink {
class WebLocalFrame;
}  // namespace blink

namespace gin {
class Arguments;
}  // namespace gin

namespace injections {

class SampleInjection : public gin::Wrappable<SampleInjection>,
                        public pal::mojom::SampleListener {
 public:
  static gin::WrapperInfo kWrapperInfo;
  static void Install(blink::WebLocalFrame* frame);
  static void Uninstall(blink::WebLocalFrame* frame);
  static void DispatchValueChanged(blink::WebLocalFrame* frame,
                                   const std::string& val);

  SampleInjection();
  SampleInjection(const SampleInjection&) = delete;
  SampleInjection& operator=(const SampleInjection&) = delete;
  ~SampleInjection() override;

  // exposed methods

  // Call function from platform. Return nothing
  void CallFunc(const std::string& arg1, const std::string& arg2);
  // Get cached value
  std::string GetValue() const;
  // Get platform value
  std::string GetPlatformValue() const;
  // Process data received from app (multiple callback objects case)
  bool ProcessData(gin::Arguments* args);

  // Subscribe "sample" object to literal data notifications
  void SubscribeToEvent();
  // Unsubscribe "sample" object from literal data notifications
  void UnsubscribeFromEvent();

  // not exposed methods
  void DispatchValueChanged(const std::string& val);

  void DataUpdated(const std::string& data) override;

private:
  static v8::MaybeLocal<v8::Object> CreateSampleObject(
      v8::Isolate* isolate,
      v8::Local<v8::Object> global);
  static void RunSupplementJS(v8::Isolate* isolate,
                              v8::Local<v8::Context> context);

  static const char kCallFuncMethodName[];
  static const char kGetPlatformValueMethodName[];
  static const char kGetValueMethodName[];
  static const char kProcessDataMethodName[];
  static const char kSubscribeToEventMethodName[];
  static const char kUnsubscribeFromEventMethodName[];

  static const char kDispatchValueChangedName[];
  static const char kReceivedSampleUpdateName[];

  gin::ObjectTemplateBuilder GetObjectTemplateBuilder(
      v8::Isolate* isolate) final;

  void OnSubscribeRespond(
      mojo::PendingAssociatedReceiver<pal::mojom::SampleListener> receiver);

  void OnProcessDataRespond(
      std::unique_ptr<v8::Persistent<v8::Function>> callback,
      std::string data,
      bool result);

  void ReceivedSampleUpdate(const std::string& value);

  std::string cached_app_value_;
  mojo::AssociatedReceiver<pal::mojom::SampleListener> listener_receiver_;
  mojo::Remote<pal::mojom::Sample> remote_sample_;
};

}  // namespace injections

#endif  // NEVA_INJECTION_RENDERER_SAMPLE_SAMPLE_INJECTION_H_
