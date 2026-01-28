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

#include "ui/ozone/platform/wayland/host/wayland_seat_manager.h"

#include "base/command_line.h"
#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "ui/events/devices/touchscreen_device.h"
#include "ui/events/event_switches.h"
#include "ui/ozone/platform/wayland/host/wayland_cursor.h"
#include "ui/ozone/platform/wayland/host/wayland_touch.h"

namespace ui {

WaylandSeatManager::WaylandSeatManager() = default;

WaylandSeatManager::~WaylandSeatManager() = default;

void WaylandSeatManager::AddSeat(WaylandConnection* connection,
                                 std::uint32_t seat_id,
                                 wl_seat* seat) {
  DCHECK(!seat_map_.count(seat_id));
  seat_map_[seat_id] = std::make_unique<WaylandSeat>(seat, connection);

  if (!first_seat_) {
    first_seat_ = seat_map_[seat_id].get();
  }
}

void WaylandSeatManager::UpdateCursorBitmap(
    const std::vector<SkBitmap>& bitmaps,
    const gfx::Point& hotspot_in_dips,
    int buffer_scale) {
  for (const auto& [id, seat] : seat_map_) {
    if (auto* cursor = seat->cursor()) {
      cursor->UpdateBitmap(bitmaps, hotspot_in_dips, buffer_scale);
    }
  }
}

void WaylandSeatManager::SetCursorPlatformShape(wl_cursor* cursor_data,
                                                int buffer_scale) {
  for (const auto& [id, seat] : seat_map_) {
    if (auto* cursor = seat->cursor()) {
      cursor->SetPlatformShape(cursor_data, buffer_scale);
    }
  }
}

void WaylandSeatManager::RefreshKeyboard() {
  for (const auto& [id, seat] : seat_map_) {
    seat->RefreshKeyboard();
  }
}

std::vector<TouchscreenDevice> WaylandSeatManager::CreateTouchscreenDevices()
    const {
  std::vector<TouchscreenDevice> devices;
  auto* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(switches::kIgnoreTouchDevices)) {
    return devices;
  }

  const int touch_points = GetMaxTouchPoints();
  for (const auto& [id, seat] : seat_map_) {
    if (const auto* touch = seat->touch()) {
      const std::string device_name = "touch-" + std::to_string(touch->id());
      devices.emplace_back(touch->id(), InputDeviceType::INPUT_DEVICE_UNKNOWN,
                           device_name, gfx::Size(), touch_points);
    }
  }

  return devices;
}

void WaylandSeatManager::UpdateCursor() {
  for (const auto& [id, seat] : seat_map_) {
    seat->UpdateCursor();
  }
}

int WaylandSeatManager::GetMaxTouchPoints() const {
  auto* command_line = base::CommandLine::ForCurrentProcess();
  std::string str =
      command_line->GetSwitchValueASCII(switches::kForceMaxTouchPoints);

  if (!str.empty()) {
    int max_touch_points;
    if (base::StringToInt(str, &max_touch_points)) {
      return max_touch_points;
    } else {
      LOG(ERROR) << "Failed to get force max touch points. Use default value.";
    }
  }

  return kDefaultMaxTouchPoints;
}

}  // namespace ui
