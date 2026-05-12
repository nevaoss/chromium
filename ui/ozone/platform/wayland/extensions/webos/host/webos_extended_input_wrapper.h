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

#ifndef UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WEBOS_EXTENDED_INPUT_WRAPPER_H_
#define UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WEBOS_EXTENDED_INPUT_WRAPPER_H_

#include "ui/ozone/platform/wayland/extensions/webos/common/wayland_webos_object.h"
#include "ui/ozone/platform/wayland/host/extended_input_wrapper.h"

namespace ui {

class WebosExtendedInputWrapper : public ExtendedInputWrapper {
 public:
  explicit WebosExtendedInputWrapper(wl_webos_xinput* xinput);
  WebosExtendedInputWrapper(const WebosExtendedInputWrapper&) = delete;
  WebosExtendedInputWrapper& operator=(const WebosExtendedInputWrapper&) =
      delete;
  ~WebosExtendedInputWrapper() override;

  // ExtendedInputWrapper
  void Activate(const std::string& type) override;
  void Deactivate() override;
  void InvokeAction(std::uint32_t keysym,
                    XInputKeySymbolType symbol_type,
                    XInputEventType event_type) override;

  // wl_webos_xinput_listener
  // Requests a client to activate an input matching the |type| method.
  static void Activated(void* data, wl_webos_xinput* xinput, const char* type);
  // Requests a client to deactivate the input.
  static void Deactivated(void* data, wl_webos_xinput* xinput);

 private:
  wl::Object<wl_webos_xinput> webos_xinput_;
};

}  // namespace ui

#endif  // UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WEBOS_EXTENDED_INPUT_WRAPPER_H_
