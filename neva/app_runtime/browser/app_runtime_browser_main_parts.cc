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

#include "neva/app_runtime/browser/app_runtime_browser_main_parts.h"

#include "base/command_line.h"
#include "base/logging.h"
#include "base/run_loop.h"
#include "base/strings/string_number_conversions.h"
#include "base/task/single_thread_task_runner.h"
#include "components/heap_profiling/multi_process/supervisor.h"
#include "components/services/heap_profiling/public/cpp/settings.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/common/result_codes.h"
#include "device/bluetooth/dbus/dbus_bluez_manager_wrapper_linux.h"
#include "net/base/network_change_notifier_factory.h"
#include "neva/app_runtime/browser/app_runtime_browser_context.h"
#include "neva/app_runtime/browser/app_runtime_browser_main_extra_parts.h"
#include "neva/app_runtime/browser/app_runtime_devtools_manager_delegate.h"
#include "neva/app_runtime/browser/net/app_runtime_network_change_notifier.h"
#include "neva/app_runtime/browser/permissions/app_runtime_permissions_client.h"
#include "neva/app_runtime/ui/app_runtime_touch_menu_runner.h"
#include "ui/linux/linux_ui.h"
#include "ui/views/widget/desktop_aura/neva/views_delegate_stub.h"

#if defined(ENABLE_PLUGINS)
#include "content/public/browser/plugin_service.h"
#endif

#if defined(USE_AURA)
#include "ui/aura/env.h"
#include "ui/base/ime/init/input_method_initializer.h"
#include "ui/display/screen.h"
#include "ui/views/widget/desktop_aura/desktop_screen.h"
#endif

#if defined(USE_OZONE)
#include "ui/ozone/public/ozone_platform.h"
#endif  // defined(USE_OZONE)

#if defined(USE_NEVA_CHROME_EXTENSIONS)
#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "components/prefs/pref_service.h"
#include "extensions/browser/api/api_browser_context_keyed_service_factories.h"
#include "extensions/browser/browser_context_keyed_service_factories.h"
#include "neva/extensions/browser/browser_context_keyed_service_factories.h"
#endif  // defined(USE_NEVA_CHROME_EXTENSIONS)

namespace neva_app_runtime {

namespace {

bool IsWayland() {
#if defined(USE_OZONE)
  return ui::OzonePlatform::IsWayland();
#else  // defined(USE_OZONE)
  return false;
#endif  // !defined(USE_OZONE)
}

}  // namespace

class AppRuntimeNetworkChangeNotifierFactory
    : public net::NetworkChangeNotifierFactory {
 public:
  // net::NetworkChangeNotifierFactory overrides.
  std::unique_ptr<net::NetworkChangeNotifier> CreateInstanceWithInitialTypes(
      net::NetworkChangeNotifier::ConnectionType /*initial_type*/,
      net::NetworkChangeNotifier::ConnectionSubtype /*initial_subtype*/)
      override {
    return base::WrapUnique(new AppRuntimeNetworkChangeNotifier());
  }
};

AppRuntimeBrowserMainParts::AppRuntimeBrowserMainParts() : BrowserMainParts() {}

AppRuntimeBrowserMainParts::~AppRuntimeBrowserMainParts() {}

void AppRuntimeBrowserMainParts::AddParts(
    AppRuntimeBrowserMainExtraParts* parts) {
  app_runtime_extra_parts_.push_back(parts);
}

int AppRuntimeBrowserMainParts::DevToolsPort() const {
  return AppRuntimeDevToolsManagerDelegate::GetHttpHandlerPort();
}

void AppRuntimeBrowserMainParts::EnableDevTools() {
  if (dev_tools_enabled_)
    return;

  AppRuntimeDevToolsManagerDelegate::StartHttpHandler(
      GetDefaultBrowserContext());
  dev_tools_enabled_ = true;
}

void AppRuntimeBrowserMainParts::DisableDevTools() {
  if (!dev_tools_enabled_)
    return;

  AppRuntimeDevToolsManagerDelegate::StopHttpHandler();
  dev_tools_enabled_ = false;
}

int AppRuntimeBrowserMainParts::PreEarlyInitialization() {
  if (!IsWayland()) {
    // Only for testing. As stub for other platforms.
    ui::InitializeInputMethodForTesting();
  }
  return RESULT_CODE_NORMAL_EXIT;
}

void AppRuntimeBrowserMainParts::ToolkitInitialized() {
  if (!views::ViewsDelegate::GetInstance()) {
    views_delegate_ = std::make_unique<views::ViewsDelegateStub>();
  }

  // Must be initialized after views_delegate_ to rewrite
  // global TouchSelectionMenuRunner reference.
  touch_selection_menu_runner_ =
      std::make_unique<AppRuntimeTouchSelectionMenuRunner>();

  if (!views::LayoutProvider::Get()) {
    layout_provider_ = std::make_unique<views::LayoutProvider>();
  }
}

void AppRuntimeBrowserMainParts::PreCreateMainMessageLoop() {
  // Replace the default NetworkChangeNotifierFactory with app runtime
  // implementation. This must be done before BrowserMainLoop calls
  // net::NetworkChangeNotifier::Create() in PostMainMessageLoopStart().
  net::NetworkChangeNotifier::SetFactory(
      new neva_app_runtime::AppRuntimeNetworkChangeNotifierFactory());
}

void AppRuntimeBrowserMainParts::PostCreateMainMessageLoop() {
  bluez::DBusBluezManagerWrapperLinux::Initialize();
#if defined(USE_OZONE)
  auto shutdown_cb = base::BindOnce([] {
    base::Process::TerminateCurrentProcessImmediately(1);
    LOG(FATAL) << "Web-engine failed to shutdown.";
  });
  ui::OzonePlatform::GetInstance()->PostCreateMainMessageLoop(
      std::move(shutdown_cb),
      base::SingleThreadTaskRunner::GetCurrentDefault());
#endif  // defined(USE_OZONE)
}

int AppRuntimeBrowserMainParts::PreMainMessageLoopRun() {
#if defined(USE_NEVA_CHROME_EXTENSIONS)
  // Extensions clients should be created before KeyedServiceFactories creation.
  extensions_client_ = std::make_unique<neva::NevaExtensionsClient>();
  extensions::ExtensionsClient::Set(extensions_client_.get());

  extensions_browser_client_ =
      std::make_unique<neva::NevaExtensionsBrowserClient>();
  extensions::ExtensionsBrowserClient::Set(extensions_browser_client_.get());

  // All KeyedServiceFactories must be created before context is created.
  extensions::EnsureBrowserContextKeyedServiceFactoriesBuilt();
  neva::EnsureBrowserContextKeyedServiceFactoriesBuilt();
#endif  // defined(USE_NEVA_CHROME_EXTENSIONS)

  auto* browser_context = GetDefaultBrowserContext();
  std::ignore = browser_context;

#if defined(ENABLE_PLUGINS)
  plugin_service_filter_.reset(new AppRuntimePluginServiceFilter);
  content::PluginService::GetInstance()->SetFilter(
      plugin_service_filter_.get());
#endif

#if defined(USE_AURA)
  if (!display::Screen::HasScreen()) {
    screen_ = views::CreateDesktopScreen();
  }

  aura::Env::GetInstance();
#endif

  for (auto* extra_part : app_runtime_extra_parts_)
    extra_part->PreMainMessageLoopRun();

  return RESULT_CODE_NORMAL_EXIT;
}

void AppRuntimeBrowserMainParts::WillRunMainMessageLoop(
      std::unique_ptr<base::RunLoop>& run_loop) {
  for (auto* extra_part : app_runtime_extra_parts_)
    extra_part->WillRunMainMessageLoop(run_loop);
}

int AppRuntimeBrowserMainParts::PreCreateThreads() {
  // Make sure permissions client has been set.
  AppRuntimePermissionsClient::GetInstance();
  return content::RESULT_CODE_NORMAL_EXIT;
}

void AppRuntimeBrowserMainParts::PostCreateThreads() {
  heap_profiling::Mode mode = heap_profiling::GetModeForStartup();
  if (mode != heap_profiling::Mode::kNone)
    heap_profiling::Supervisor::GetInstance()->Start(base::NullCallback());

  for (auto* extra_part : app_runtime_extra_parts_) {
    extra_part->PostCreateThreads();
  }
}

void AppRuntimeBrowserMainParts::PostMainMessageLoopRun() {
  DisableDevTools();
  AppRuntimeBrowserContext::Clear();
}

void AppRuntimeBrowserMainParts::PostDestroyThreads() {
  bluez::DBusBluezManagerWrapperLinux::Shutdown();
}

content::BrowserContext*
AppRuntimeBrowserMainParts::GetDefaultBrowserContext() const {
  return AppRuntimeBrowserContext::From("");
}

}  // namespace neva_app_runtime
