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

#ifndef NEVA_INJECTION_RENDERER_MEMORYMANAGER_MEMORYMANAGER_INJECTION_H_
#define NEVA_INJECTION_RENDERER_MEMORYMANAGER_MEMORYMANAGER_INJECTION_H_

#include "gin/object_template_builder.h"
#include "gin/wrappable.h"
#include "mojo/public/cpp/bindings/associated_receiver.h"
#include "mojo/public/cpp/bindings/pending_associated_receiver.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "neva/injection/renderer/injection_browser_control_base.h"
#include "neva/pal_service/public/mojom/memorymanager.mojom.h"
#include "v8/include/v8.h"

namespace blink {
class WebLocalFrame;
}  // namespace blink

namespace gin {
class Arguments;
}  // namespace gin

namespace injections {

class MemoryManagerInjection : public gin::Wrappable<MemoryManagerInjection>,
                               public pal::mojom::MemoryManagerListener {
 public:
  static gin::WrapperInfo kWrapperInfo;
  static const char kGetMemoryStatusMethodName[];
  static const char kOnLevelChangedCallbackName[];
  static void Install(blink::WebLocalFrame* frame);
  static void Uninstall(blink::WebLocalFrame* frame);

  MemoryManagerInjection();
  MemoryManagerInjection(const MemoryManagerInjection&) = delete;
  MemoryManagerInjection& operator=(const MemoryManagerInjection&) = delete;
  ~MemoryManagerInjection() override;
  void GetMemoryStatus(gin::Arguments* args);

 private:
  static v8::MaybeLocal<v8::Object> CreateObject(v8::Isolate* isolate,
                                                 v8::Local<v8::Object> parent);

  gin::ObjectTemplateBuilder GetObjectTemplateBuilder(
      v8::Isolate* isolate) final;

  void LevelChanged(const std::string& value) override;

  void SubscribeToLevelChanged();
  void OnSubscribeRespond(
      mojo::PendingAssociatedReceiver<pal::mojom::MemoryManagerListener>
          receiver);
  void OnGetMemoryStatusRespond(
      std::unique_ptr<v8::Persistent<v8::Function>> callback,
      const std::string& status);

  mojo::AssociatedReceiver<pal::mojom::MemoryManagerListener>
      listener_receiver_;
  mojo::Remote<pal::mojom::MemoryManager> remote_memorymanager_;
};

}  // namespace injections

#endif  // NEVA_INJECTION_RENDERER_MEMORYMANAGER_MEMORYMANAGER_INJECTION_H_
