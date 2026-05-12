// Copyright 2018 LG Electronics, Inc.
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

#ifndef UI_AURA_NEVA_WINDOW_TREE_HOST_H_
#define UI_AURA_NEVA_WINDOW_TREE_HOST_H_

#include <string>
#include <vector>

#include "neva/app_runtime/public/app_runtime_constants.h"
#include "ui/aura/aura_export.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/platform_window/neva/xinput_types.h"
#include "ui/views/widget/desktop_aura/neva/ui_constants.h"

#if defined(IS_AGL)
#include "ui/aura/neva/agl/window_tree_host_agl.h"
#endif

namespace ui {
class EventHandler;
class WindowGroupConfiguration;
}

namespace aura {
namespace neva {

class AURA_EXPORT WindowTreeHost
#if defined(IS_AGL)
    : public WindowTreeHostAgl
#endif
{
 public:
  WindowTreeHost() = default;
  WindowTreeHost(const WindowTreeHost&) = delete;
  WindowTreeHost& operator=(const WindowTreeHost&) = delete;
  virtual ~WindowTreeHost() = default;

  virtual void AddPreTargetHandler(ui::EventHandler* handler) {}
  virtual void RemovePreTargetHandler(ui::EventHandler* handler) {}
  virtual void CompositorResumeDrawing() {}
  virtual void SetCustomCursor(neva_app_runtime::CustomCursorType type,
                               const std::string& path,
                               int hotspot_x,
                               int hotspot_y) {}
  virtual void SetCursorVisibility(bool visible) {}
  virtual void SetInputRegion(const std::vector<gfx::Rect>& region) {}
  virtual void SetGroupKeyMask(ui::KeyMask key_mask) {}
  virtual void SetKeyMask(ui::KeyMask key_mask, bool set) {}
  virtual void SetUseVirtualKeyboard(bool enable) {}
  virtual void SetWindowProperty(const std::string& name,
                                 const std::string& value) {}
  virtual void SetFullscreen(bool fullscreen, int64_t target_display_id) {}
  virtual void XInputActivate(const std::string& type) {}
  virtual void XInputDeactivate() {}
  virtual void XInputInvokeAction(uint32_t keysym,
                                  ui::XInputKeySymbolType symbol_type,
                                  ui::XInputEventType event_type) {}
  virtual void CreateGroup(const ui::WindowGroupConfiguration& config) {}
  virtual void AttachToGroup(const std::string& name,
                             const std::string& layer) {}
  virtual void FocusGroupOwner() {}
  virtual void FocusGroupLayer() {}
  virtual void DetachGroup() {}
};

}  // namespace neva
}  // namespace aura

#endif  // UI_AURA_NEVA_WINDOW_TREE_HOST_H_
