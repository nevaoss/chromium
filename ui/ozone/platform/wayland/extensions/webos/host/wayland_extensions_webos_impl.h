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

#ifndef UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WAYLAND_EXTENSIONS_WEBOS_IMPL_H_
#define UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WAYLAND_EXTENSIONS_WEBOS_IMPL_H_

#include <memory>

#include "ui/ozone/platform/wayland/extensions/webos/common/wayland_webos_object.h"
#include "ui/ozone/platform/wayland/extensions/webos/host/wayland_extensions_webos.h"
#include "ui/ozone/platform/wayland/extensions/webos/host/wayland_window_webos.h"
#include "ui/ozone/platform/wayland/extensions/webos/host/webos_extended_input_wrapper.h"
#include "ui/ozone/platform/wayland/extensions/webos/host/webos_input_manager_wrapper.h"
#include "ui/ozone/platform/wayland/extensions/webos/host/webos_input_panel_manager.h"
#include "ui/ozone/platform/wayland/extensions/webos/host/webos_surface_group_compositor_wrapper.h"
#include "ui/ozone/platform/wayland/extensions/webos/host/webos_text_model_factory_wrapper.h"
#include "ui/ozone/platform/wayland/host/wayland_extensions.h"

namespace ui {

class WaylandExtensionsWebosImpl : public WaylandExtensions,
                                   public WaylandExtensionsWebos {
 public:
  explicit WaylandExtensionsWebosImpl(WaylandConnection* connection);
  WaylandExtensionsWebosImpl(const WaylandExtensionsWebosImpl&) = delete;
  WaylandExtensionsWebosImpl& operator=(const WaylandExtensionsWebosImpl&) = delete;
  ~WaylandExtensionsWebosImpl() override;

  // WaylandExtensions overrides
  bool Bind(wl_registry* registry,
            uint32_t name,
            const char* interface,
            uint32_t version) override;

  bool HasShellObject() const override;

  std::unique_ptr<ShellToplevelWrapper> CreateShellToplevel(
      WaylandWindow* window) override;

  std::unique_ptr<ShellPopupWrapper> CreateShellPopup(
      WaylandWindow* window) override;

  std::unique_ptr<WaylandWindow> CreateWaylandWindow(
      PlatformWindowDelegate* delegate,
      WaylandConnection* connection) override;

  // WaylandExtensionsWebos overrides
  ExtendedInputWrapper* GetExtendedInput() override;
  InputManagerWrapper* GetInputManager() override;
  InputPanelManager* GetInputPanelManager() override;
  SurfaceGroupCompositorWrapper* GetSurfaceGroupCompositor() override;
  WebosTextModelFactoryWrapper* GetWebosTextModelFactory() override;

  wl_shell* shell() const override { return wl_shell_.get(); }
  wl_webos_shell* webos_shell() const override { return webos_shell_.get(); }

  WaylandConnection* connection() const { return connection_; }

 private:
  wl::Object<wl_shell> wl_shell_;
  wl::Object<wl_webos_shell> webos_shell_;

  WaylandConnection* const connection_;

  std::unique_ptr<WebosExtendedInputWrapper> extended_input_;
  std::unique_ptr<WebosInputManagerWrapper> input_manager_;
  std::unique_ptr<WebosInputPanelManager> input_panel_manager_;
  std::unique_ptr<WebosSurfaceGroupCompositorWrapper> surface_group_compositor_;
  std::unique_ptr<WebosTextModelFactoryWrapper> text_model_factory_;
};

}  // namespace ui

#endif  // UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WAYLAND_EXTENSIONS_WEBOS_IMPL_H_
