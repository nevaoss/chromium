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

#ifndef UI_AURA_NEVA_WINDOW_TREE_HOST_PLATFORM_H_
#define UI_AURA_NEVA_WINDOW_TREE_HOST_PLATFORM_H_

#include <memory>

#include "ui/aura/aura_export.h"

#if defined(IS_AGL)
#include "ui/aura/neva/agl/window_tree_host_platform_agl.h"
#else
#include "ui/aura/window_tree_host.h"
#endif

namespace ui {
class EventHandler;
}

namespace aura {

class Window;
class WindowTreeHostPlatform;

namespace neva {

class AURA_EXPORT WindowTreeHostPlatform
// TODO : Add webOS window tree host platform
#if defined(IS_AGL)
    : public WindowTreeHostPlatformAgl
#else
    : public aura::WindowTreeHost
#endif
{
 public:
  WindowTreeHostPlatform(
      std::unique_ptr<Window> window,
      aura::WindowTreeHostPlatform* window_tree_host_platform)
#if defined(IS_AGL)
      : WindowTreeHostPlatformAgl(std::move(window),
                                  window_tree_host_platform) {}
#else
      : aura::WindowTreeHost(std::move(window)) {}
#endif
  WindowTreeHostPlatform(const WindowTreeHostPlatform&) = delete;
  WindowTreeHostPlatform& operator=(const WindowTreeHostPlatform&) = delete;
  ~WindowTreeHostPlatform() override = default;

  // TODO : Add the rest of common Neva app runtime methods from neva::WindowTreeHost

  // Overridden from neva::WindowTreeHost
  void AddPreTargetHandler(ui::EventHandler* handler) override;
  void RemovePreTargetHandler(ui::EventHandler* handler) override;
};

}  // namespace neva
}  // namespace aura

#endif  // UI_AURA_NEVA_WINDOW_TREE_HOST_PLATFORM_H_
