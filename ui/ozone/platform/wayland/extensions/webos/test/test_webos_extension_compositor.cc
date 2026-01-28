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

#include "ui/ozone/platform/wayland/extensions/webos/test/test_webos_extension_compositor.h"

#include <wayland-server-core.h>

#include "ui/ozone/platform/wayland/extensions/webos/test/mock_webos_extension_surface.h"
#include "ui/ozone/platform/wayland/test/server_object.h"
#include "ui/ozone/platform/wayland/test/test_region.h"

namespace wl {

namespace {

constexpr uint32_t kCompositorVersion = 1;

void CreateSurface(wl_client* client,
                   wl_resource* compositor_resource,
                   uint32_t id) {
  wl_resource* resource =
      CreateResourceWithImpl<::testing::NiceMock<MockWebosExtensionSurface>>(
          client, &wl_surface_interface,
          wl_resource_get_version(compositor_resource),
          &kMockWebosExtensionSurfaceImpl, id);
  GetUserDataAs<TestWebosExtensionCompositor>(compositor_resource)
      ->AddSurface(GetUserDataAs<MockWebosExtensionSurface>(resource));
}

void CreateRegion(wl_client* client, wl_resource* resource, uint32_t id) {
  wl_resource* region_resource =
      wl_resource_create(client, &wl_region_interface, 1, id);
  SetImplementation(region_resource, &kTestWlRegionImpl,
                    std::make_unique<TestRegion>());
}

}  // namespace

const struct wl_compositor_interface kTestWebosExtensionCompositorImpl = {
    CreateSurface,  // create_surface
    CreateRegion,   // create_region
};

TestWebosExtensionCompositor::TestWebosExtensionCompositor()
    : GlobalObject(&wl_compositor_interface,
                   &kTestWebosExtensionCompositorImpl,
                   kCompositorVersion) {}

TestWebosExtensionCompositor::~TestWebosExtensionCompositor() {}

void TestWebosExtensionCompositor::AddSurface(
    MockWebosExtensionSurface* surface) {
  surfaces_.push_back(surface);
}

}  // namespace wl
