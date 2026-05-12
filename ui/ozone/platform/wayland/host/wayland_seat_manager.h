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

#ifndef UI_OZONE_PLATFORM_WAYLAND_HOST_WAYLAND_SEAT_MANAGER__H_
#define UI_OZONE_PLATFORM_WAYLAND_HOST_WAYLAND_SEAT_MANAGER__H_

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

#include "base/memory/raw_ptr.h"
#include "ui/ozone/platform/wayland/host/wayland_seat.h"

class SkBitmap;

struct wl_cursor;
struct wl_seat;

namespace gfx {
class Point;
}

namespace ui {

struct TouchscreenDevice;
class WaylandConnection;

// Stores WaylandSeat wrapper objects.
class WaylandSeatManager {
 public:
  WaylandSeatManager();
  WaylandSeatManager(const WaylandSeatManager&) = delete;
  WaylandSeatManager& operator=(const WaylandSeatManager&) = delete;
  virtual ~WaylandSeatManager();

  // Adds Wayland |seat| with the |seat_id| to the storage.
  void AddSeat(WaylandConnection* connection,
               std::uint32_t seat_id,
               wl_seat* seat);
  // Returns the very first WaylandSeat wrapper.
  WaylandSeat* GetFirstSeat() const { return first_seat_; }

  // Updates bitmaps for all cursors within the storage.
  void UpdateCursorBitmap(const std::vector<SkBitmap>& bitmaps,
                          const gfx::Point& hotspot_in_dips,
                          int buffer_scale);

  // Applies externally managed default platform data to all cursors in storage.
  void SetCursorPlatformShape(wl_cursor* cursor_data, int buffer_scale);

  // Creates or re-creates the keyboard object for all seats within the storage.
  void RefreshKeyboard();

  // Create an array of each seat's touch-screen device.
  std::vector<TouchscreenDevice> CreateTouchscreenDevices() const;

  // Updates cursor related objects in all seats.
  void UpdateCursor();

 private:
  static constexpr int kDefaultMaxTouchPoints = 1;

  int GetMaxTouchPoints() const;

  using SeatMap =
      std::unordered_map<std::uint32_t, std::unique_ptr<WaylandSeat>>;
  SeatMap seat_map_;
  raw_ptr<WaylandSeat> first_seat_ = nullptr;
};

}  // namespace ui

#endif  // UI_OZONE_PLATFORM_WAYLAND_HOST_WAYLAND_SEAT_MANAGER__H_
