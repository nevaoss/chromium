// Copyright 2021 LG Electronics, Inc.
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

#ifndef UI_AURA_NEVA_AGL_WINDOW_TREE_HOST_PLATFORM_AGL_H_
#define UI_AURA_NEVA_AGL_WINDOW_TREE_HOST_PLATFORM_AGL_H_

#include <memory>

#include "ui/aura/aura_export.h"
#include "ui/aura/window_tree_host.h"

namespace aura {

class Window;
class WindowTreeHostPlatform;

namespace neva {

class AURA_EXPORT WindowTreeHostPlatformAgl : public aura::WindowTreeHost {
 public:
  explicit WindowTreeHostPlatformAgl(
      std::unique_ptr<Window> window,
      aura::WindowTreeHostPlatform* window_tree_host_platform);
  WindowTreeHostPlatformAgl(const WindowTreeHostPlatformAgl&) = delete;
  WindowTreeHostPlatformAgl& operator=(const WindowTreeHostPlatformAgl&) = delete;
  ~WindowTreeHostPlatformAgl() override = default;

  void SetAglActivateApp(const std::string& app) override;
  void SetAglAppId(const std::string& title) override;
  void SetAglReady() override;
  void SetAglBackground() override;
  void SetAglPanel(uint32_t edge) override;

 private:
  aura::WindowTreeHostPlatform* window_tree_host_platform_;
};

}  // namespace neva
}  // namespace aura

#endif  // UI_AURA_NEVA_AGL_WINDOW_TREE_HOST_PLATFORM_AGL_H_