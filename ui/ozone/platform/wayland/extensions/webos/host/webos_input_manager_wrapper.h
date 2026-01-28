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

#ifndef UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WEBOS_INPUT_MANAGER_WRAPPER_H_
#define UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WEBOS_INPUT_MANAGER_WRAPPER_H_

#include <cstdint>

#include "ui/ozone/platform/wayland/extensions/webos/common/wayland_webos_object.h"
#include "ui/ozone/platform/wayland/host/input_manager_wrapper.h"

namespace ui {

class WaylandConnection;
class ExtendedSeatWrapper;

class WebosInputManagerWrapper : public InputManagerWrapper {
 public:
  WebosInputManagerWrapper(wl_webos_input_manager* input_manager,
                           WaylandConnection* connection);
  WebosInputManagerWrapper(const WebosInputManagerWrapper&) = delete;
  WebosInputManagerWrapper& operator=(const WebosInputManagerWrapper&) = delete;
  ~WebosInputManagerWrapper() override;

  // InputManagerWrapper
  std::unique_ptr<ExtendedSeatWrapper> GetExtendedSeat(wl_seat* seat) override;
  void SetCursorVisibility(bool is_visible) override;

  // wl_webos_input_manager_listener
  // Notifies when current cursor |visibility| is changed.
  static void CursorVisibility(void* data,
                               wl_webos_input_manager* input_manager,
                               std::uint32_t visibility,
                               wl_webos_seat* seat);

 private:
  wl::Object<wl_webos_input_manager> webos_input_manager_;

  WaylandConnection* const connection_;
};

}  // namespace ui

#endif  // UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WEBOS_INPUT_MANAGER_WRAPPER_H_
