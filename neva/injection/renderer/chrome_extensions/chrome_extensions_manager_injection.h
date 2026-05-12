// Copyright 2024 LG Electronics, Inc.
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

#ifndef NEVA_INJECTION_RENDERER_CHROME_EXTENSIONS_CHROME_EXTENSIONS_MANAGER_INJECTION_H_
#define NEVA_INJECTION_RENDERER_CHROME_EXTENSIONS_CHROME_EXTENSIONS_MANAGER_INJECTION_H_

#include "gin/object_template_builder.h"
#include "gin/wrappable.h"
#include "mojo/public/cpp/bindings/associated_receiver.h"
#include "mojo/public/cpp/bindings/pending_associated_receiver.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "neva/extensions/common/mojom/neva_extensions_services_manager.mojom.h"
#include "neva/injection/renderer/injection_events_emitter.h"
#include "v8/include/v8.h"

namespace blink {
class WebLocalFrame;
}

namespace injections {
class ChromeExtensionsInjection;

class ChromeExtensionsManagerInjection
    : public gin::Wrappable<ChromeExtensionsManagerInjection>,
      public InjectionEventsEmitter<ChromeExtensionsManagerInjection>,
      public neva::mojom::NevaExtensionsServicesManagerClient {
 public:
  static gin::WrapperInfo kWrapperInfo;

  static char kGetExtensionsServiceForMethodName[];

  static void Install(blink::WebLocalFrame* frame);
  static void Uninstall(blink::WebLocalFrame* frame);

  ChromeExtensionsManagerInjection(v8::Isolate* isolate);
  ChromeExtensionsManagerInjection(const ChromeExtensionsManagerInjection&) =
      delete;
  ChromeExtensionsManagerInjection& operator=(
      const ChromeExtensionsManagerInjection&) = delete;
  ~ChromeExtensionsManagerInjection() override;

  // mojom
  void OnExtensionServiceRegistered(const std::string& partition) override;

 private:
  gin::ObjectTemplateBuilder GetObjectTemplateBuilder(
      v8::Isolate* isolate) final;

  v8::Local<v8::Object> GetExtensionsServiceFor(const std::string& partition);

  std::map<std::string, v8::Global<v8::Object>> services_map_;

  mojo::Remote<neva::mojom::NevaExtensionsServicesManager> remote_;
  mojo::AssociatedReceiver<neva::mojom::NevaExtensionsServicesManagerClient>
      receiver_;
};

}  // namespace injections

#endif  //  NEVA_INJECTION_RENDERER_CHROME_EXTENSIONS_CHROME_EXTENSIONS_MANAGER_INJECTION_H_
