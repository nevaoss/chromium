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

#ifndef UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_TEST_WEBOS_EXTENSION_TEST_H_
#define UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_TEST_WEBOS_EXTENSION_TEST_H_

#include <memory>

#include "base/test/task_environment.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/ozone/platform/wayland/extensions/webos/test/test_webos_extension_server_thread.h"
#include "ui/ozone/platform/wayland/gpu/wayland_buffer_manager_gpu.h"
#include "ui/ozone/platform/wayland/gpu/wayland_surface_factory.h"
#include "ui/ozone/platform/wayland/host/wayland_connection.h"
#include "ui/ozone/platform/wayland/host/wayland_window.h"
#include "ui/ozone/test/mock_platform_window_delegate.h"

namespace wl {
class MockWebosExtensionSurface;
}  // namespace wl

namespace ui {

class WaylandScreen;

// WebosExtensionTest is a base class that sets up a display, window, test
// server and provides an easy synchronization between them.
class WebosExtensionTest : public ::testing::Test {
 public:
  WebosExtensionTest();
  ~WebosExtensionTest() override;

  void SetUp() override;
  void TearDown() override;

  void Sync();

  WebosExtensionTest(const WebosExtensionTest&) = delete;
  WebosExtensionTest& operator=(const WebosExtensionTest&) = delete;

 protected:
  base::test::TaskEnvironment task_environment_;

  wl::TestWebosExtensionServerThread server_;

  MockPlatformWindowDelegate delegate_;
  std::unique_ptr<WaylandSurfaceFactory> surface_factory_;
  std::unique_ptr<WaylandBufferManagerGpu> buffer_manager_gpu_;
  std::unique_ptr<WaylandConnection> connection_;
  std::unique_ptr<WaylandScreen> screen_;
  std::unique_ptr<WaylandWindow> window_;
  gfx::AcceleratedWidget widget_ = gfx::kNullAcceleratedWidget;

 private:
  bool initialized_ = false;
};

}  // namespace ui

#endif  // UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_TEST_WEBOS_EXTENSION_TEST_H_
