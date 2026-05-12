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

#include <wayland-webos-shell-server-protocol.h>

#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/ozone/platform/wayland/extensions/webos/host/wayland_shell_surface_wrapper.h"
#include "ui/ozone/platform/wayland/extensions/webos/test/mock_shell_surface.h"
#include "ui/ozone/platform/wayland/extensions/webos/test/mock_webos_extension_surface.h"
#include "ui/ozone/platform/wayland/extensions/webos/test/webos_extension_test.h"

using ::testing::Eq;
using ::testing::StrEq;

namespace ui {

class WaylandShellSurfaceWrapperTest : public WebosExtensionTest {
 public:
  WaylandShellSurfaceWrapperTest() = default;

  void SetUp() override {
    WebosExtensionTest::SetUp();

    uint32_t id = window_->root_surface()->get_surface_id();
    surface_ = server_.GetObject<wl::MockWebosExtensionSurface>(id);
    ASSERT_TRUE(surface_);
  }

  wl::MockShellSurface* GetShellSurface() { return surface_->shell_surface(); }

 private:
  wl::MockWebosExtensionSurface* surface_;
};

TEST_F(WaylandShellSurfaceWrapperTest, SetTitle) {
  EXPECT_CALL(*GetShellSurface(), SetTitle(StrEq("title")));

  window_->SetTitle(u"title");
}

}  // namespace ui
