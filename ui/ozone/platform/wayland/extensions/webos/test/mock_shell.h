// Copyright 2019 LG Electronics, Inc.
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

#ifndef UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_TEST_MOCK_SHELL_H_
#define UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_TEST_MOCK_SHELL_H_

#include <wayland-server-protocol.h>
#include <wayland-webos-shell-server-protocol.h>

#include "testing/gmock/include/gmock/gmock.h"
#include "ui/ozone/platform/wayland/test/global_object.h"

namespace wl {

extern const struct wl_shell_interface kMockShellImpl;
extern const struct wl_webos_shell_interface kMockWebosShellImpl;

// Manages shell object.
class MockShell : public GlobalObject {
 public:
  MockShell();
  ~MockShell() override;

  MockShell(const MockShell&) = delete;
  MockShell& operator=(const MockShell&) = delete;
};

// Manages webos_shell object.
class MockWebosShell : public GlobalObject {
 public:
  MockWebosShell();
  ~MockWebosShell() override;

  MockWebosShell(const MockWebosShell&) = delete;
  MockWebosShell& operator=(const MockWebosShell&) = delete;
};

}  // namespace wl

#endif  // UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_TEST_MOCK_SHELL_H_
