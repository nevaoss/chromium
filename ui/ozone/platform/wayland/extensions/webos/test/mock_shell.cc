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

#include "ui/ozone/platform/wayland/extensions/webos/test/mock_shell.h"

#include "ui/ozone/platform/wayland/extensions/webos/test/mock_webos_extension_surface.h"
#include "ui/ozone/platform/wayland/test/server_object.h"

namespace wl {

namespace {

constexpr uint32_t kShellVersion = 1;
constexpr uint32_t kWebosShellVersion = 1;

void GetShellSurfaceImpl(wl_client* client,
                         wl_resource* resource,
                         uint32_t id,
                         wl_resource* surface_resource,
                         const struct wl_interface* interface,
                         const void* implementation) {
  auto* surface = GetUserDataAs<MockWebosExtensionSurface>(surface_resource);
  bool is_shell_surface_impl = implementation == &kMockShellSurfaceImpl;
  bool already_has_a_surface = is_shell_surface_impl
                                   ? !!surface->shell_surface()
                                   : !!surface->webos_shell_surface();
  if (already_has_a_surface) {
    wl_resource_post_error(resource, WL_SHELL_ERROR_ROLE,
                           "surface already has a role");
    return;
  }
  wl_resource* shell_surface_resource = wl_resource_create(
      client, interface, wl_resource_get_version(resource), id);
  if (!shell_surface_resource) {
    wl_client_post_no_memory(client);
    return;
  }
  if (is_shell_surface_impl) {
    surface->set_shell_surface(
        std::make_unique<MockShellSurface>(shell_surface_resource));
  } else {
    surface->set_webos_shell_surface(
        std::make_unique<MockWebosShellSurface>(shell_surface_resource));
  }
}

void GetShellSurface(wl_client* client,
                     wl_resource* resource,
                     uint32_t id,
                     wl_resource* surface_resource) {
  GetShellSurfaceImpl(client, resource, id, surface_resource,
                      &wl_shell_surface_interface, &kMockShellSurfaceImpl);
}

void GetWebosShellSurface(wl_client* client,
                          wl_resource* resource,
                          uint32_t id,
                          wl_resource* surface_resource) {
  GetShellSurfaceImpl(client, resource, id, surface_resource,
                      &wl_webos_shell_surface_interface,
                      &kMockWebosShellSurfaceImpl);
}

}  // namespace

const struct wl_shell_interface kMockShellImpl = {
    &GetShellSurface,  // get_shell_surface
};

const struct wl_webos_shell_interface kMockWebosShellImpl = {
    nullptr,                // get_system_pip
    &GetWebosShellSurface,  // get_shell_surface
};

MockShell::MockShell()
    : GlobalObject(&wl_shell_interface, &kMockShellImpl, kShellVersion) {}

MockShell::~MockShell() {}

MockWebosShell::MockWebosShell()
    : GlobalObject(&wl_webos_shell_interface,
                   &kMockWebosShellImpl,
                   kWebosShellVersion) {}

MockWebosShell::~MockWebosShell() {}

}  // namespace wl
