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

#include "ui/ozone/platform/wayland/extensions/webos/test/mock_webos_extension_surface.h"

namespace wl {

namespace {

void Attach(wl_client* client,
            wl_resource* resource,
            wl_resource* buffer_resource,
            int32_t x,
            int32_t y) {
  auto* surface = GetUserDataAs<MockWebosExtensionSurface>(resource);
  surface->AttachNewBuffer(buffer_resource, x, y);
}

void SetOpaqueRegion(wl_client* client,
                     wl_resource* resource,
                     wl_resource* region) {
  GetUserDataAs<MockWebosExtensionSurface>(resource)->SetOpaqueRegion(region);
}

void SetInputRegion(wl_client* client,
                    wl_resource* resource,
                    wl_resource* region) {
  GetUserDataAs<MockWebosExtensionSurface>(resource)->SetInputRegion(region);
}

void Damage(wl_client* client,
            wl_resource* resource,
            int32_t x,
            int32_t y,
            int32_t width,
            int32_t height) {
  GetUserDataAs<MockWebosExtensionSurface>(resource)->Damage(x, y, width,
                                                             height);
}

void Frame(struct wl_client* client,
           struct wl_resource* resource,
           uint32_t callback) {
  auto* surface = GetUserDataAs<MockWebosExtensionSurface>(resource);

  wl_resource* callback_resource =
      wl_resource_create(client, &wl_callback_interface, 1, callback);
  surface->set_frame_callback(callback_resource);

  surface->Frame(callback);
}

void Commit(wl_client* client, wl_resource* resource) {
  GetUserDataAs<MockWebosExtensionSurface>(resource)->Commit();
}

void SetBufferScale(wl_client* client, wl_resource* resource, int32_t scale) {
  GetUserDataAs<MockWebosExtensionSurface>(resource)->SetBufferScale(scale);
}

void DamageBuffer(struct wl_client* client,
                  struct wl_resource* resource,
                  int32_t x,
                  int32_t y,
                  int32_t width,
                  int32_t height) {
  GetUserDataAs<MockWebosExtensionSurface>(resource)->DamageBuffer(x, y, width,
                                                                   height);
}

}  // namespace

const struct wl_surface_interface kMockWebosExtensionSurfaceImpl = {
    DestroyResource,  // destroy
    Attach,           // attach
    Damage,           // damage
    Frame,            // frame
    SetOpaqueRegion,  // set_opaque_region
    SetInputRegion,   // set_input_region
    Commit,           // commit
    nullptr,          // set_buffer_transform
    SetBufferScale,   // set_buffer_scale
    DamageBuffer,     // damage_buffer
};

MockWebosExtensionSurface::MockWebosExtensionSurface(wl_resource* resource)
    : ServerObject(resource) {}

MockWebosExtensionSurface::~MockWebosExtensionSurface() {
  if (shell_surface_ && shell_surface_->resource())
    wl_resource_destroy(shell_surface_->resource());
}

MockWebosExtensionSurface* MockWebosExtensionSurface::FromResource(
    wl_resource* resource) {
  if (!ResourceHasImplementation(resource, &wl_surface_interface,
                                 &kMockWebosExtensionSurfaceImpl))
    return nullptr;
  return GetUserDataAs<MockWebosExtensionSurface>(resource);
}

void MockWebosExtensionSurface::AttachNewBuffer(wl_resource* buffer_resource,
                                                int32_t x,
                                                int32_t y) {
  if (attached_buffer_) {
    DCHECK(!prev_attached_buffer_);
    prev_attached_buffer_ = attached_buffer_;
  }
  attached_buffer_ = buffer_resource;

  Attach(buffer_resource, x, y);
}

void MockWebosExtensionSurface::ReleasePrevAttachedBuffer() {
  if (!prev_attached_buffer_)
    return;

  wl_buffer_send_release(prev_attached_buffer_);
  wl_client_flush(wl_resource_get_client(prev_attached_buffer_));
  prev_attached_buffer_ = nullptr;
}

void MockWebosExtensionSurface::SendFrameCallback() {
  if (!frame_callback_)
    return;

  wl_callback_send_done(
      frame_callback_,
      0 /* trequest-specific data for the callback. not used */);
  wl_client_flush(wl_resource_get_client(frame_callback_));
  wl_resource_destroy(frame_callback_);
  frame_callback_ = nullptr;
}

}  // namespace wl
