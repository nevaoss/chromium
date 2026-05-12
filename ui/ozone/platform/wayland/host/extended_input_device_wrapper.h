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

#ifndef UI_OZONE_PLATFORM_WAYLAND_HOST_EXTENDED_INPUT_DEVICE_WRAPPER_H_
#define UI_OZONE_PLATFORM_WAYLAND_HOST_EXTENDED_INPUT_DEVICE_WRAPPER_H_

namespace ui {

// Wrapper for the extended input device (e.g., sensor) interface.
class ExtendedInputDeviceWrapper {
 public:
  virtual ~ExtendedInputDeviceWrapper() = default;

  // Requests to enable/disable sending data for the device.
  virtual void RequestData(bool is_enabled) = 0;
};

}  // namespace ui

#endif  // UI_OZONE_PLATFORM_WAYLAND_HOST_EXTENDED_INPUT_DEVICE_WRAPPER_H_
