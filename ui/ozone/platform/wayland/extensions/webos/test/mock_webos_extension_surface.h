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

#ifndef UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_TEST_MOCK_WEBOS_EXTENSION_SURFACE_H_
#define UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_TEST_MOCK_WEBOS_EXTENSION_SURFACE_H_

#include <memory>
#include <utility>

#include <wayland-server-protocol.h>

#include "base/check.h"
#include "base/logging.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "ui/ozone/platform/wayland/extensions/webos/test/mock_shell_surface.h"
#include "ui/ozone/platform/wayland/test/server_object.h"

struct wl_resource;

namespace wl {

extern const struct wl_surface_interface kMockWebosExtensionSurfaceImpl;

// Manages client surfaces.
class MockWebosExtensionSurface : public ServerObject {
 public:
  explicit MockWebosExtensionSurface(wl_resource* resource);
  ~MockWebosExtensionSurface() override;

  static MockWebosExtensionSurface* FromResource(wl_resource* resource);

  MOCK_METHOD3(Attach, void(wl_resource* buffer, int32_t x, int32_t y));
  MOCK_METHOD1(SetOpaqueRegion, void(wl_resource* region));
  MOCK_METHOD1(SetInputRegion, void(wl_resource* region));
  MOCK_METHOD1(Frame, void(uint32_t callback));
  MOCK_METHOD4(Damage,
               void(int32_t x, int32_t y, int32_t width, int32_t height));
  MOCK_METHOD0(Commit, void());
  MOCK_METHOD1(SetBufferScale, void(int32_t scale));
  MOCK_METHOD4(DamageBuffer,
               void(int32_t x, int32_t y, int32_t width, int32_t height));

  void set_shell_surface(std::unique_ptr<MockShellSurface> surface) {
    shell_surface_ = std::move(surface);
  }
  MockShellSurface* shell_surface() const { return shell_surface_.get(); }

  void set_webos_shell_surface(std::unique_ptr<MockWebosShellSurface> surface) {
    webos_shell_surface_ = std::move(surface);
  }
  MockWebosShellSurface* webos_shell_surface() const {
    return webos_shell_surface_.get();
  }

  void set_frame_callback(wl_resource* callback_resource) {
    DCHECK(!frame_callback_);
    frame_callback_ = callback_resource;
  }

  void AttachNewBuffer(wl_resource* buffer_resource, int32_t x, int32_t y);
  void ReleasePrevAttachedBuffer();
  void SendFrameCallback();

  MockWebosExtensionSurface(const MockWebosExtensionSurface&) = delete;
  MockWebosExtensionSurface& operator=(const MockWebosExtensionSurface&) =
      delete;

 private:
  std::unique_ptr<MockShellSurface> shell_surface_;
  std::unique_ptr<MockWebosShellSurface> webos_shell_surface_;

  wl_resource* frame_callback_ = nullptr;

  wl_resource* attached_buffer_ = nullptr;
  wl_resource* prev_attached_buffer_ = nullptr;
};

}  // namespace wl

#endif  // UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_TEST_MOCK_WEBOS_EXTENSION_SURFACE_H_
