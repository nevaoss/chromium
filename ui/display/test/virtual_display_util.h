// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_DISPLAY_TEST_VIRTUAL_DISPLAY_UTIL_H_
#define UI_DISPLAY_TEST_VIRTUAL_DISPLAY_UTIL_H_

#include <memory>

#include "ui/display/display_observer.h"
#include "ui/gfx/geometry/size.h"
#include "ui/gfx/geometry/vector2d.h"

namespace display {
class Screen;

namespace test {

struct DISPLAY_EXPORT DisplayParams {
  gfx::Size resolution;
  gfx::Vector2d dpi = gfx::Vector2d(96, 96);
  std::string description;
};

// TODO(neva): Remove this workaround when Neva GCC version upgraded to 12+.
#if defined(__GNUC__) && (__GNUC__ < 12) && !defined(__clang__)
bool operator<(const display::test::DisplayParams& a,
               const display::test::DisplayParams& b) {
#else  // defined(__GNUC__) && (__GNUC__ < 12) && !defined(__clang__)
bool constexpr operator<(const display::test::DisplayParams& a,
                         const display::test::DisplayParams& b) {
#endif // !(defined(__GNUC__) && (__GNUC__ < 12) && !defined(__clang__))
  return std::tuple(a.resolution.width(), a.resolution.height(), a.dpi.x(),
                    a.dpi.y(), a.description) <
         std::tuple(b.resolution.width(), b.resolution.height(), b.dpi.x(),
                    b.dpi.y(), b.description);
}

// TODO(neva): Remove this workaround when Neva GCC version upgraded to 12+.
#if defined(__GNUC__) && (__GNUC__ < 12) && !defined(__clang__)
bool operator==(const display::test::DisplayParams& a,
                const display::test::DisplayParams& b) {
#else  // defined(__GNUC__) && (__GNUC__ < 12) && !defined(__clang__)
bool constexpr operator==(const display::test::DisplayParams& a,
                          const display::test::DisplayParams& b) {
#endif // !(defined(__GNUC__) && (__GNUC__ < 12) && !defined(__clang__))
  return a.resolution == b.resolution && a.dpi == b.dpi &&
         a.description == b.description;
}

// This interface creates system-level virtual displays to support the automated
// integration testing of display information and window management APIs in
// multi-screen device environments. It updates displays that display::Screen
// impl sees, but is generally not compatible with `TestScreen` subclasses.
class VirtualDisplayUtil {
 public:
  virtual ~VirtualDisplayUtil() = default;

  // Creates an instance for the current platform if available. Returns nullptr
  // on unsupported platforms (i.e. not implemented, or host is missing
  // requirements).
  static std::unique_ptr<VirtualDisplayUtil> TryCreate(Screen* screen);

  // Adds a virtual display and returns the generated display::Display id, which
  // can be used with the Screen instance or passed to `RemoveDisplay`.
  virtual int64_t AddDisplay(const DisplayParams& display_params) = 0;
  // Remove a virtual display corresponding to the specified display ID.
  virtual void RemoveDisplay(int64_t display_id) = 0;
  // Remove all added virtual displays.
  virtual void ResetDisplays() = 0;

  // Supported Display configurations.
// TODO(neva): Remove this workaround when Neva GCC version upgraded to 12+.
#if defined(__GNUC__) && (__GNUC__ < 12) && !defined(__clang__)
  static const DisplayParams k1920x1080;
  static const DisplayParams k1024x768;
#else  // defined(__GNUC__) && (__GNUC__ < 12) && !defined(__clang__)
  static constexpr DisplayParams k1920x1080 = {gfx::Size(1920, 1080)};
  static constexpr DisplayParams k1024x768 = {gfx::Size(1024, 768)};
#endif  // !(defined(__GNUC__) && (__GNUC__ < 12) && !defined(__clang__))
};

}  // namespace test
}  // namespace display

#endif  // UI_DISPLAY_TEST_VIRTUAL_DISPLAY_UTIL_H_
