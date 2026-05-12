// Copyright 2019 LG Electronics, Inc.
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

#ifndef NEVA_INJECTION_RENDERER_NETWORK_ERROR_PAGE_CONTROLLER_INJECTION_H_
#define NEVA_INJECTION_RENDERER_NETWORK_ERROR_PAGE_CONTROLLER_INJECTION_H_

#include "gin/object_template_builder.h"
#include "gin/wrappable.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "neva/pal_service/public/mojom/network_error_page_controller.mojom.h"
#include "v8/include/v8.h"

namespace blink {
class WebLocalFrame;
}  // namespace blink

namespace injections {

class NetworkErrorPageControllerInjection
    : public gin::Wrappable<NetworkErrorPageControllerInjection> {
 public:
  static gin::WrapperInfo kWrapperInfo;
  static void Install(blink::WebLocalFrame* frame);
  static void Uninstall(blink::WebLocalFrame* frame);
  static const char kExposedName[];

  explicit NetworkErrorPageControllerInjection();
  ~NetworkErrorPageControllerInjection() override;

  // Execute a "NETWORK SETTINGS"/"SETTINGS" button click.
  bool SettingsButtonClick(int target_id);
  // Close native shell (BrowserShell) if the action is defined.
  void CloseShellIfNeeded();

 private:
  gin::ObjectTemplateBuilder GetObjectTemplateBuilder(
      v8::Isolate* isolate) override;

  mojo::Remote<pal::mojom::NetworkErrorPageController> controller_;
};

}  // namespace injections

#endif  // NEVA_INJECTION_RENDERER_NETWORK_ERROR_PAGE_CONTROLLER_INJECTION_H_
