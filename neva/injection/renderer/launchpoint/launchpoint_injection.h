// Copyright 2025 LG Electronics, Inc.
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

#ifndef NEVA_INJECTION_RENDERER_LAUNCHPOINT_INJECTION_H_
#define NEVA_INJECTION_RENDERER_LAUNCHPOINT_INJECTION_H_

#include "gin/object_template_builder.h"
#include "gin/persistent.h"
#include "gin/weak_cell.h"
#include "gin/wrappable.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "neva/pal_service/public/mojom/launchpoint.mojom.h"
#include "v8/include/v8-cppgc.h"
#include "v8/include/v8.h"

namespace blink {
class WebLocalFrame;
}  // namespace blink

namespace gin {
class Arguments;
}  // namespace gin

namespace injections {

class LaunchPointInjection final : public gin::Wrappable<LaunchPointInjection> {
 public:
  static constexpr gin::WrapperInfo kWrapperInfo = {{gin::kEmbedderNativeGin},
                                                    gin::kLaunchPointInjection};

  static const char kLaunchPointObjectName[];
  static const char kAddLaunchPointMethodName[];

  static void Install(blink::WebLocalFrame* frame);
  static void Uninstall(blink::WebLocalFrame* frame);

  LaunchPointInjection();
  LaunchPointInjection(const LaunchPointInjection&) = delete;
  LaunchPointInjection& operator=(const LaunchPointInjection&) = delete;
  ~LaunchPointInjection() override;

  cppgc::Persistent<gin::WeakCell<LaunchPointInjection>> GetAsWeak() {
    return gin::WrapPersistent(weak_factory_.GetWeakCell(
        v8::Isolate::GetCurrent()->GetCppHeap()->GetAllocationHandle()));
  }

  void AddLaunchPoint(gin::Arguments* args);

  // gin::Wrappable : v8::Object::Wrappable
  void Trace(cppgc::Visitor* visitor) const final;

 private:
  // gin::Wrappable
  const gin::WrapperInfo* wrapper_info() const override;

  gin::ObjectTemplateBuilder GetObjectTemplateBuilder(
      v8::Isolate* isolate) final;

  void OnAddLaunchPointRespond(v8::Global<v8::Function> callback,
                               bool return_value,
                               const std::string& launch_point_id,
                               int error_code,
                               const std::string& error_text);

  mojo::Remote<pal::mojom::LaunchPoint> remote_launchpoint_;
  gin::WeakCellFactory<LaunchPointInjection> weak_factory_{this};
};

}  // namespace injections

#endif  // NEVA_INJECTION_RENDERER_LAUNCHPOINT_INJECTION_H_
