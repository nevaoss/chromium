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

#include "ui/ozone/platform/wayland/extensions/webos/host/webos_seat_wrapper.h"

#include <wayland-webos-input-manager-client-protocol.h>

#include "base/check.h"
#include "ui/ozone/platform/wayland/extensions/webos/common/wayland_webos_util.h"
#include "ui/ozone/platform/wayland/extensions/webos/host/webos_accelerometer_wrapper.h"
#include "ui/ozone/platform/wayland/extensions/webos/host/webos_gyroscope_wrapper.h"

namespace ui {

namespace {

// Bitmask of the webOS seat capabilities.
enum class Capability : std::uint32_t {
  kDefault = 0,
  kGyroscope = 1,
  kAccelerometer = 2
};

}  // namespace

WebosSeatWrapper::WebosSeatWrapper(wl_webos_seat* seat) {
  webos_seat_.reset(seat);

  static const wl_webos_seat_listener webos_seat_listener = {
      WebosSeatWrapper::Info};

  wl_webos_seat_add_listener(webos_seat_.get(), &webos_seat_listener, this);
}

WebosSeatWrapper::~WebosSeatWrapper() = default;

std::unique_ptr<ExtendedInputDeviceWrapper>
WebosSeatWrapper::GetAccelerometer() {
  if (capabilities_ & Capability::kAccelerometer) {
    wl_webos_accelerometer* webos_accelerometer =
        wl_webos_seat_get_accelerometer(webos_seat_.get());

    return std::make_unique<WebosAccelerometerWrapper>(webos_accelerometer);
  }

  return nullptr;
}

std::unique_ptr<ExtendedInputDeviceWrapper> WebosSeatWrapper::GetGyroscope() {
  if (capabilities_ & Capability::kGyroscope) {
    wl_webos_gyroscope* webos_gyroscope =
        wl_webos_seat_get_gyroscope(webos_seat_.get());

    return std::make_unique<WebosGyroscopeWrapper>(webos_gyroscope);
  }

  return nullptr;
}

// static
void WebosSeatWrapper::Info(void* data,
                            wl_webos_seat* seat,
                            std::uint32_t id,
                            const char* name,
                            std::uint32_t designator,
                            std::uint32_t capabilities) {
  WebosSeatWrapper* webos_seat_wrapper = static_cast<WebosSeatWrapper*>(data);
  DCHECK(webos_seat_wrapper);

  webos_seat_wrapper->capabilities_ = capabilities;
}

}  // namespace ui
