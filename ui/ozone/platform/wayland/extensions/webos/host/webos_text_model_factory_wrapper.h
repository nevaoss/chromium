// Copyright 2020 LG Electronics, Inc.
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
//

#ifndef UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WEBOS_TEXT_MODEL_FACTORY_WRAPPER_H_
#define UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WEBOS_TEXT_MODEL_FACTORY_WRAPPER_H_

#include <memory>

#include "ui/ozone/platform/wayland/extensions/webos/common/wayland_webos_object.h"

namespace ui {

class WaylandConnection;
class WaylandWindowWebos;
class WebosInputPanel;
class WebosTextModelWrapper;

// Wrapper for the Wayland |text_model_factory| interface used to create text
// model instances.
class WebosTextModelFactoryWrapper {
 public:
  explicit WebosTextModelFactoryWrapper(text_model_factory* factory);
  WebosTextModelFactoryWrapper(const WebosTextModelFactoryWrapper&) = delete;
  WebosTextModelFactoryWrapper& operator=(const WebosTextModelFactoryWrapper&) =
      delete;
  ~WebosTextModelFactoryWrapper() = default;

  // Returns a newly created text model wrapper.
  std::unique_ptr<WebosTextModelWrapper> CreateTextModel(
      WebosInputPanel* input_panel,
      WaylandConnection* connection,
      WaylandWindowWebos* window);

 private:
  wl::Object<text_model_factory> webos_text_model_factory_;
};

}  // namespace ui

#endif  // UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WEBOS_TEXT_MODEL_FACTORY_WRAPPER_H_
