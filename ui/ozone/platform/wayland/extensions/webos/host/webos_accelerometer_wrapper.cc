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

#include "ui/ozone/platform/wayland/extensions/webos/host/webos_accelerometer_wrapper.h"

#include <cstdint>

#include <wayland-webos-input-manager-client-protocol.h>

#include "base/notreached.h"

namespace ui {

namespace {

constexpr std::uint32_t kEnable = 1;
constexpr std::uint32_t kDisable = 0;

}  // namespace

WebosAccelerometerWrapper::WebosAccelerometerWrapper(
    wl_webos_accelerometer* accelerometer) {
  webos_accelerometer_.reset(accelerometer);

  static const wl_webos_accelerometer_listener webos_accelerometer_listener = {
      WebosAccelerometerWrapper::SensorData};

  wl_webos_accelerometer_add_listener(webos_accelerometer_.get(),
                                      &webos_accelerometer_listener, this);
}

WebosAccelerometerWrapper::~WebosAccelerometerWrapper() = default;

void WebosAccelerometerWrapper::RequestData(bool is_enabled) {
  wl_webos_accelerometer_request_data(webos_accelerometer_.get(),
                                      is_enabled ? kEnable : kDisable);
}

// static
void WebosAccelerometerWrapper::SensorData(
    void* data,
    wl_webos_accelerometer* accelerometer,
    wl_fixed_t x,
    wl_fixed_t y,
    wl_fixed_t z) {
  NOTIMPLEMENTED_LOG_ONCE();
}

}  // namespace ui
