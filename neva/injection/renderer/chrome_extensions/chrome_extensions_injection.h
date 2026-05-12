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

#ifndef NEVA_INJECTION_RENDERER_CHROME_EXTENSIONS_CHROME_EXTENSIONS_INJECTION_H_
#define NEVA_INJECTION_RENDERER_CHROME_EXTENSIONS_CHROME_EXTENSIONS_INJECTION_H_

#include "gin/object_template_builder.h"
#include "gin/wrappable.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "neva/extensions/common/mojom/neva_extensions_service.mojom.h"
#include "neva/injection/renderer/injection_events_emitter.h"
#include "v8/include/v8.h"

namespace blink {
class WebLocalFrame;
}

namespace injections {

class ChromeExtensionsInjection
    : public gin::Wrappable<ChromeExtensionsInjection>,
      public InjectionEventsEmitter<ChromeExtensionsInjection>,
      public neva::mojom::NevaExtensionsServiceClient {
 public:
  static gin::WrapperInfo kWrapperInfo;

  ChromeExtensionsInjection(
      v8::Isolate* isolate,
      mojo::Remote<neva::mojom::NevaExtensionsService> remote);
  ChromeExtensionsInjection(const ChromeExtensionsInjection&) = delete;
  ChromeExtensionsInjection& operator=(const ChromeExtensionsInjection&) =
      delete;
  ~ChromeExtensionsInjection() override;

  void RunAddEventListener(gin::Arguments* args);
  void GetExtensionsInfo(gin::Arguments* args);
  void OnExtensionsInfo(std::unique_ptr<v8::Persistent<v8::Function>> callback,
                        std::vector<base::Value> infos);
  void SelectExtension(uint64_t tab_id, const std::string& extension_id);
  void OnExtensionTabCreated(gin::Arguments* args);
  void OnExtensionTabClosed(uint64_t tab_id);
  void OnExtensionTabActivated(uint64_t tab_id);
  void OnExtensionTabUpdated(uint64_t tab_id, const std::string& change_info);
  void OnExtensionPopupViewCreated(uint64_t popup_view_id, uint64_t tab_id);

  // mojom::NevaExtensionsServiceClient
  void OnExtensionTabCreationRequested(uint64_t request_id,
                                       const std::string& tab_type) override;
  void OnExtensionTabCloseRequested(uint64_t tab_id) override;
  void OnExtensionTabFocusRequested(uint64_t tab_id) override;
  void OnExtensionPopupCreationRequested() override;
  void OnExtensionPopupCloseRequested(uint64_t popup_view_id) override;
  void OnExtensionPopupUpdateRequested(uint64_t popup_view_id,
                                       int width,
                                       int height) override;

 private:
  gin::ObjectTemplateBuilder GetObjectTemplateBuilder(
      v8::Isolate* isolate) final;

  std::vector<std::pair<uint64_t, std::string>> pending_tab_creation_requests_;

  mojo::Remote<neva::mojom::NevaExtensionsService> remote_;
  mojo::Receiver<neva::mojom::NevaExtensionsServiceClient> receiver_{this};
};

}  // namespace injections

#endif  // NEVA_INJECTION_RENDERER_CHROME_EXTENSIONS_CHROME_EXTENSIONS_INJECTION_H_
