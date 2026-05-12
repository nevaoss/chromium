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

#ifndef UI_OZONE_PLATFORM_WAYLAND_HOST_EXTENDED_INPUT_WRAPPER_H_
#define UI_OZONE_PLATFORM_WAYLAND_HOST_EXTENDED_INPUT_WRAPPER_H_

#include <cstdint>
#include <string>

#include "ui/platform_window/neva/xinput_types.h"

namespace ui {

// Wrapper for the extended input interface used to allow a client
// to pass an input event to another client.
class ExtendedInputWrapper {
 public:
  virtual ~ExtendedInputWrapper() = default;

  // Notifies the compositor this input has been activated.
  virtual void Activate(const std::string& type) = 0;

  // Notifies the compositor this input has been deactivated.
  virtual void Deactivate() = 0;

  // Invokes an action by passing the |keysym|.
  virtual void InvokeAction(std::uint32_t keysym,
                            XInputKeySymbolType symbol_type,
                            XInputEventType event_type) = 0;
};

}  // namespace ui

#endif  // UI_OZONE_PLATFORM_WAYLAND_HOST_EXTENDED_INPUT_WRAPPER_H_
