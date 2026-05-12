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

#include "ui/ozone/platform/wayland/extensions/webos/host/webos_text_model_factory_wrapper.h"

#include <wayland-text-client-protocol.h>

#include "ui/ozone/platform/wayland/extensions/webos/host/wayland_window_webos.h"
#include "ui/ozone/platform/wayland/extensions/webos/host/webos_text_model_wrapper.h"
#include "ui/ozone/platform/wayland/host/wayland_connection.h"
#include "ui/ozone/platform/wayland/host/wayland_seat.h"

#if defined(USE_NEVA_APPRUNTIME)
#include "ui/ozone/platform/wayland/host/wayland_seat_manager.h"
#endif  // defined(USE_NEVA_APPRUNTIME)

namespace ui {

WebosTextModelFactoryWrapper::WebosTextModelFactoryWrapper(
    text_model_factory* factory)
    : webos_text_model_factory_(factory) {}

std::unique_ptr<WebosTextModelWrapper>
WebosTextModelFactoryWrapper::CreateTextModel(WebosInputPanel* input_panel,
                                              WaylandConnection* connection,
                                              WaylandWindowWebos* window) {
  text_model* text_model =
      text_model_factory_create_text_model(webos_text_model_factory_.get());

  if (text_model && connection && connection->seat()) {
    return std::make_unique<WebosTextModelWrapper>(text_model, input_panel,
                                                   connection->seat(), window);
  }

  return nullptr;
}

}  // namespace ui
