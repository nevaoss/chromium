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

#ifndef UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WEBOS_SEAT_WRAPPER_H_
#define UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WEBOS_SEAT_WRAPPER_H_

#include <cstdint>

#include "ui/ozone/platform/wayland/extensions/webos/common/wayland_webos_object.h"
#include "ui/ozone/platform/wayland/host/extended_seat_wrapper.h"

namespace ui {

class ExtendedInputDeviceWrapper;

class WebosSeatWrapper : public ExtendedSeatWrapper {
 public:
  explicit WebosSeatWrapper(wl_webos_seat* seat);
  WebosSeatWrapper(const WebosSeatWrapper&) = delete;
  WebosSeatWrapper& operator=(const WebosSeatWrapper&) = delete;
  ~WebosSeatWrapper() override;

  // ExtendedSeatWrapper
  std::unique_ptr<ExtendedInputDeviceWrapper> GetAccelerometer() override;
  std::unique_ptr<ExtendedInputDeviceWrapper> GetGyroscope() override;

  // wl_webos_seat_listener
  // Sent immediately upon retrieval of the webOS seat from the input manager.
  static void Info(void* data,
                   wl_webos_seat* seat,
                   std::uint32_t id,
                   const char* name,
                   std::uint32_t designator,
                   std::uint32_t capabilities);

 private:
  wl::Object<wl_webos_seat> webos_seat_;

  std::uint32_t capabilities_;
};

}  // namespace ui

#endif  // UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WEBOS_SEAT_WRAPPER_H_
