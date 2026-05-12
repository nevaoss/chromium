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

#ifndef UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_TEST_TEST_WEBOS_EXTENSION_COMPOSITOR_H_
#define UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_TEST_TEST_WEBOS_EXTENSION_COMPOSITOR_H_

#include <vector>

#include "ui/ozone/platform/wayland/test/global_object.h"

namespace wl {

class MockWebosExtensionSurface;

// Manages wl_compositor object.
class TestWebosExtensionCompositor : public GlobalObject {
 public:
  TestWebosExtensionCompositor();
  ~TestWebosExtensionCompositor() override;

  void AddSurface(MockWebosExtensionSurface* surface);

  TestWebosExtensionCompositor(const TestWebosExtensionCompositor&) = delete;
  TestWebosExtensionCompositor& operator=(const TestWebosExtensionCompositor&) =
      delete;

 private:
  std::vector<MockWebosExtensionSurface*> surfaces_;
};

}  // namespace wl

#endif  // UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_TEST_TEST_WEBOS_EXTENSION_COMPOSITOR_H_
