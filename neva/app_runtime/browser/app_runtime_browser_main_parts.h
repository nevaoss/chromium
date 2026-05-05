// Copyright 2016 LG Electronics, Inc.
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

#ifndef NEVA_APP_RUNTIME_BROWSER_APP_RUNTIME_BROWSER_MAIN_PARTS_H_
#define NEVA_APP_RUNTIME_BROWSER_APP_RUNTIME_BROWSER_MAIN_PARTS_H_

#include "content/public/browser/browser_main_parts.h"
#include "neva/app_runtime/public/webapp_window_base.h"
#include "neva/app_runtime/public/webview_base.h"
#include "ui/views/layout/layout_provider.h"

#if defined(ENABLE_PLUGINS)
#include "neva/app_runtime/browser/app_runtime_plugin_service_filter.h"
#endif

#if defined(USE_NEVA_CHROME_EXTENSIONS)
#include "neva/extensions/browser/neva_extension_system.h"
#include "neva/extensions/browser/neva_extensions_browser_client.h"
#include "neva/extensions/common/neva_extensions_client.h"

class PrefService;
#endif  // defined(USE_NEVA_CHROME_EXTENSIONS)

namespace devtools_http_handler {
class DevToolsHttpHandler;
}  // namespace devtools_http_handler

#if defined(USE_AURA)
namespace display {
class Screen;
}
#endif

namespace content {
class BrowserContext;
}

namespace views {
class ViewsDelegateStub;
}

namespace neva_app_runtime {

class AppRuntimeBrowserMainExtraParts;
class AppRuntimeRemoteDebuggingServer;
class AppRuntimeTouchSelectionMenuRunner;

class AppRuntimeBrowserMainParts : public content::BrowserMainParts {
 public:
  AppRuntimeBrowserMainParts();
  ~AppRuntimeBrowserMainParts() override;

  void AddParts(AppRuntimeBrowserMainExtraParts* parts);
  int DevToolsPort() const;
  void EnableDevTools();
  void DisableDevTools();

  // content::BrowserMainParts
  int PreEarlyInitialization() override;
  void ToolkitInitialized() override;
  void PreCreateMainMessageLoop() override;
  void PostCreateMainMessageLoop() override;
  int PreCreateThreads() override;
  void PostCreateThreads() override;
  int PreMainMessageLoopRun() override;
  void WillRunMainMessageLoop(
      std::unique_ptr<base::RunLoop>& run_loop) override;
  void PostMainMessageLoopRun() override;
  void PostDestroyThreads() override;

  content::BrowserContext* GetDefaultBrowserContext() const;

 private:
  bool dev_tools_enabled_ = false;

#if defined(ENABLE_PLUGINS)
  std::unique_ptr<AppRuntimePluginServiceFilter> plugin_service_filter_;
#endif
#if defined(USE_AURA)
  std::unique_ptr<display::Screen> screen_;
#endif
  std::vector<AppRuntimeBrowserMainExtraParts*> app_runtime_extra_parts_;
  std::unique_ptr<views::ViewsDelegateStub> views_delegate_;
  std::unique_ptr<AppRuntimeTouchSelectionMenuRunner>
      touch_selection_menu_runner_;

  std::unique_ptr<views::LayoutProvider> layout_provider_;

#if defined(USE_NEVA_CHROME_EXTENSIONS)
  raw_ptr<neva::NevaExtensionSystem> extension_system_;
  std::unique_ptr<neva::NevaExtensionsClient> extensions_client_;
  std::unique_ptr<neva::NevaExtensionsBrowserClient> extensions_browser_client_;

  std::unique_ptr<PrefService> local_state_;
  std::unique_ptr<PrefService> user_pref_service_;
#endif  // defined(USE_NEVA_CHROME_EXTENSIONS)
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_BROWSER_APP_RUNTIME_BROWSER_MAIN_PARTS_H_
