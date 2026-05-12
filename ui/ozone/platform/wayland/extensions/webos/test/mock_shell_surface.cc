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

#include "base/strings/utf_string_conversions.h"
#include "ui/ozone/platform/wayland/extensions/webos/test/mock_shell_surface.h"

namespace wl {

namespace {

void Pong(wl_client* client, wl_resource* resource, uint32_t serial) {
  GetUserDataAs<MockShellSurface>(resource)->Pong(serial);
}

void SetTopLevel(wl_client* client, wl_resource* resource) {
}
void SetTitle(wl_client* client, wl_resource* resource, const char* title) {
  auto* mock = GetUserDataAs<MockShellSurface>(resource);
  mock->set_title(title);
  mock->SetTitle(mock->title());
}

void SetState(wl_client* client, wl_resource* resource, uint32_t state) {
  GetUserDataAs<MockWebosShellSurface>(resource)->SetState(state);
}

void SetProperty(wl_client* client,
                 wl_resource* resource,
                 const char* name,
                 const char* value) {
  GetUserDataAs<MockWebosShellSurface>(resource)->SetProperty(name, value);
}

}  // namespace

const struct wl_shell_surface_interface kMockShellSurfaceImpl = {
    &Pong,           // pong
    nullptr,         // move
    nullptr,         // resize
    &SetTopLevel,    // set_toplevel
    nullptr,         // set_transient
    nullptr,         // set_fullscreen
    nullptr,         // set_popup
    nullptr,         // set_maximized
    &SetTitle,       // set_title
    nullptr,         // set_class
};

const struct wl_webos_shell_surface_interface kMockWebosShellSurfaceImpl = {
    nullptr,       // set_location_hint
    &SetState,     // set_state
    &SetProperty,  // set_property
    nullptr,       // set_keymask
};

MockShellSurface::MockShellSurface(wl_resource* resource)
    : ServerObject(resource) {
  SetImplementationUnretained(resource, &kMockShellSurfaceImpl, this);
}

MockShellSurface::~MockShellSurface() {}

MockWebosShellSurface::MockWebosShellSurface(wl_resource* resource)
    : ServerObject(resource) {
  SetImplementationUnretained(resource, &kMockWebosShellSurfaceImpl, this);
}

MockWebosShellSurface::~MockWebosShellSurface() {}

}  // namespace wl
