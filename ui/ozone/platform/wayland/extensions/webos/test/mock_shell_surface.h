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

#ifndef UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_TEST_MOCK_SHELL_SURFACE_H_
#define UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_TEST_MOCK_SHELL_SURFACE_H_

#include <memory>
#include <utility>

#include <wayland-server-protocol.h>
#include <wayland-webos-shell-server-protocol.h>

#include "testing/gmock/include/gmock/gmock.h"
#include "ui/ozone/platform/wayland/test/server_object.h"

struct wl_resource;

namespace wl {

extern const struct wl_shell_surface_interface kMockShellSurfaceImpl;
extern const struct wl_webos_shell_surface_interface kMockWebosShellSurfaceImpl;

// Manages wl_shell_surface_interface.
class MockShellSurface : public ServerObject {
 public:
  MockShellSurface(wl_resource* resource);
  ~MockShellSurface() override;

  MockShellSurface(const MockShellSurface&) = delete;
  MockShellSurface& operator=(const MockShellSurface&) = delete;

  // These mock methods are specific to the wl_shell_surface_interface
  // interface.
  MOCK_METHOD1(Pong, void(uint32_t serial));
  MOCK_METHOD1(SetTitle, void(const std::string& title));

  std::string title() const { return title_; }
  void set_title(const char* title) { title_ = std::string(title); }

private:
  std::string title_;
};

// Manages wl_webos_shell_surface_interface.
class MockWebosShellSurface : public ServerObject {
 public:
  MockWebosShellSurface(wl_resource* resource);
  ~MockWebosShellSurface() override;

  // These mock methods are specific for the wl_webos_shell_surface_interface
  // interface.
  MOCK_METHOD1(SetState, void(uint32_t state));
  MOCK_METHOD2(SetProperty, void(const char* name, const char* value));

  MockWebosShellSurface(const MockWebosShellSurface&) = delete;
  MockWebosShellSurface& operator=(const MockWebosShellSurface&) = delete;
};

}  // namespace wl

#endif  // UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_TEST_MOCK_SHELL_SURFACE_H_
