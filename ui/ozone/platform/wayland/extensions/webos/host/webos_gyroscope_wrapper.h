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

#ifndef UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WEBOS_GYROSCOPE_WRAPPER_H_
#define UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WEBOS_GYROSCOPE_WRAPPER_H_

#include <wayland-util.h>

#include "ui/ozone/platform/wayland/extensions/webos/common/wayland_webos_object.h"
#include "ui/ozone/platform/wayland/host/extended_input_device_wrapper.h"

namespace ui {

class WebosGyroscopeWrapper : public ExtendedInputDeviceWrapper {
 public:
  explicit WebosGyroscopeWrapper(wl_webos_gyroscope* gyroscope);
  WebosGyroscopeWrapper(const WebosGyroscopeWrapper&) = delete;
  WebosGyroscopeWrapper& operator=(const WebosGyroscopeWrapper&) = delete;
  ~WebosGyroscopeWrapper() override;

  // ExtendedInputDeviceWrapper
  void RequestData(bool is_enabled) override;

  // wl_webos_gyroscope_listener
  // Sent upon sending data for a gyroscope device is explicitly requested.
  static void SensorData(void* data,
                         wl_webos_gyroscope* gyroscope,
                         wl_fixed_t x,
                         wl_fixed_t y,
                         wl_fixed_t z);

 private:
  wl::Object<wl_webos_gyroscope> webos_gyroscope_;
};

}  // namespace ui

#endif  // UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WEBOS_GYROSCOPE_WRAPPER_H_
