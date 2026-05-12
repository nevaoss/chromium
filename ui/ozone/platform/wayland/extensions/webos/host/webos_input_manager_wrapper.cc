// Copyright 2020 LG Electronics, Inc.
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

#include "ui/ozone/platform/wayland/extensions/webos/host/webos_input_manager_wrapper.h"

#include <wayland-webos-input-manager-client-protocol.h>

#include "base/logging.h"
#include "ui/ozone/platform/wayland/extensions/webos/host/wayland_window_webos.h"
#include "ui/ozone/platform/wayland/extensions/webos/host/webos_seat_wrapper.h"
#include "ui/ozone/platform/wayland/host/wayland_connection.h"
#include "ui/ozone/platform/wayland/host/wayland_window_manager.h"

namespace ui {

namespace {

constexpr std::uint32_t kVisible = 1;
constexpr std::uint32_t kInvisible = 0;

}  // namespace

WebosInputManagerWrapper::WebosInputManagerWrapper(
    wl_webos_input_manager* input_manager,
    WaylandConnection* connection)
    : connection_(connection) {
  webos_input_manager_.reset(input_manager);

  static const wl_webos_input_manager_listener webos_input_manager_listener = {
      WebosInputManagerWrapper::CursorVisibility};

  wl_webos_input_manager_add_listener(webos_input_manager_.get(),
                                      &webos_input_manager_listener, this);
}

WebosInputManagerWrapper::~WebosInputManagerWrapper() = default;

std::unique_ptr<ExtendedSeatWrapper> WebosInputManagerWrapper::GetExtendedSeat(
    wl_seat* seat) {
  wl_webos_seat* webos_seat =
      wl_webos_input_manager_get_webos_seat(webos_input_manager_.get(), seat);

  return std::make_unique<WebosSeatWrapper>(webos_seat);
}

void WebosInputManagerWrapper::SetCursorVisibility(bool is_visible) {
  wl_webos_input_manager_set_cursor_visibility(
      webos_input_manager_.get(), is_visible ? kVisible : kInvisible);
}

// static
void WebosInputManagerWrapper::CursorVisibility(
    void* data,
    wl_webos_input_manager* input_manager,
    std::uint32_t visibility,
    wl_webos_seat* seat) {
  WebosInputManagerWrapper* input_manager_wrapper =
      static_cast<WebosInputManagerWrapper*>(data);
  DCHECK(input_manager_wrapper);
  DCHECK(input_manager_wrapper->connection_);

  WaylandWindowManager* window_manager =
      input_manager_wrapper->connection_->window_manager();

  if (window_manager) {
    auto webos_window = static_cast<WaylandWindowWebos*>(
        window_manager->GetCurrentKeyboardFocusedWindow());

    if (webos_window)
      webos_window->HandleCursorVisibilityChanged(visibility ? true : false);
  }
}

}  // namespace ui
