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

#include "ui/ozone/platform/wayland/extensions/webos/test/webos_extension_test.h"

#include "base/run_loop.h"
#include "ui/ozone/platform/wayland/extensions/webos/test/mock_webos_extension_surface.h"
#include "ui/ozone/platform/wayland/host/wayland_output_manager.h"
#include "ui/ozone/platform/wayland/host/wayland_screen.h"
#include "ui/platform_window/platform_window_init_properties.h"

using ::testing::_;
using ::testing::SaveArg;

namespace ui {

WebosExtensionTest::WebosExtensionTest()
    : task_environment_(base::test::TaskEnvironment::MainThreadType::UI) {
  connection_ = std::make_unique<WaylandConnection>();
  buffer_manager_gpu_ = std::make_unique<WaylandBufferManagerGpu>();
  surface_factory_ = std::make_unique<WaylandSurfaceFactory>(
      connection_.get(), buffer_manager_gpu_.get());
}

WebosExtensionTest::~WebosExtensionTest() {}

void WebosExtensionTest::SetUp() {
  ASSERT_TRUE(server_.Start());
  ASSERT_TRUE(connection_->Initialize());
  screen_ = connection_->wayland_output_manager()->CreateWaylandScreen();
  EXPECT_CALL(delegate_, OnAcceleratedWidgetAvailable(_))
      .WillOnce(SaveArg<0>(&widget_));
  PlatformWindowInitProperties properties;
  properties.bounds = gfx::Rect(0, 0, 800, 600);
  properties.type = PlatformWindowType::kWindow;
  window_ = WaylandWindow::Create(&delegate_, connection_.get(),
                                  std::move(properties));
  ASSERT_NE(widget_, gfx::kNullAcceleratedWidget);

  window_->Show(false);

  // Wait for the client to flush all pending requests from initialization.
  base::RunLoop().RunUntilIdle();

  // Pause the server after it has responded to all incoming events.
  server_.Pause();

  initialized_ = true;
}

void WebosExtensionTest::TearDown() {
  if (initialized_)
    Sync();
}

void WebosExtensionTest::Sync() {
  // Resume the server, flushing its pending events.
  server_.Resume();

  // Wait for the client to finish processing these events.
  base::RunLoop().RunUntilIdle();

  // Pause the server, after it has finished processing any follow-up requests
  // from the client.
  server_.Pause();
}

}  // namespace ui
