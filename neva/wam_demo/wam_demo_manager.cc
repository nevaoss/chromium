// Copyright 2020 LG Electronics, Inc.
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

#include "neva/wam_demo/wam_demo_manager.h"

#include <signal.h>

#include <sstream>
#include <tuple>

#include "base/command_line.h"
#include "base/logging.h"
#include "build/build_config.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/content_switches.h"
#include "neva/app_runtime/browser/net/app_runtime_network_change_notifier.h"
#include "neva/app_runtime/public/webapp_window_base.h"
#include "neva/app_runtime/public/webview_base.h"
#include "neva/app_runtime/public/window_group_configuration.h"
#include "neva/app_runtime/webview_profile.h"
#include "neva/pal_service/pal_platform_factory.h"
#include "neva/pal_service/public/platform_system_delegate.h"
#include "neva/wam_demo/wam_demo_app_launch_params.h"
#include "neva/wam_demo/wam_demo_application.h"
#include "neva/wam_demo/wam_demo_manager_delegate.h"
#include "neva/wam_demo/wam_demo_switches.h"
#include "neva/wam_demo/wam_demo_webview.h"
#include "neva/wam_demo/wam_demo_window.h"
#include "util.h"

#if defined(USE_OZONE)
#include "ui/ozone/public/ozone_platform.h"
#endif  // defined(USE_OZONE)

#if BUILDFLAG(IS_WEBOS)
#include "base/files/file_util.h"
#include "base/path_service.h"
#endif  // BUILDFLAG(IS_WEBOS)

namespace wam_demo {

namespace {

const char kDefaultLayerName[] = "com.webos.app.neva.wam.demo.default";
const int kDefaultWindowWidth = 640;
const int kDefaultWindowHeight = 480;
const int kDefaultFullscreenWidth = 1920;
const int kDefaultFullscreenHeight = 1080;

bool IsWayland() {
#if defined(USE_OZONE)
  return ui::OzonePlatform::IsWayland();
#else  // defined(USE_OZONE)
  return false;
#endif  // !defined(USE_OZONE)
}

#if BUILDFLAG(IS_WEBOS)
std::string GetMediaCodecCapability() {
  base::FilePath path;
  std::string media_codec_capability;
  if (base::PathService::Get(base::FILE_MEDIA_CODEC_CAPABILITIES, &path) &&
      base::PathExists(path) &&
      base::ReadFileToString(path, &media_codec_capability)) {
    return media_codec_capability;
  }
  return std::string();
}
#endif  // BUILDFLAG(IS_WEBOS)

}  // namespace

WamDemoManager::WamDemoManager(WamDemoManagerDelegate* delegate)
    : system_delegate_(
          pal::PlatformFactory::Get()->CreatePlatformSystemDelegate()),
      delegate_(delegate) {}

WamDemoManager::~WamDemoManager() = default;

void WamDemoManager::LaunchApplicationFromCLI(const std::string& appid,
                                              const std::string& appurl,
                                              bool fullscreen,
                                              bool frameless,
                                              const std::string& profile_name) {
  std::string system_appid = GetSystemAppId(appid);
  neva_app_runtime::WebAppWindowBase::CreateParams params;
  params.web_contents = nullptr;
  params.type =
      neva_app_runtime::WebAppWindowBase::CreateParams::WidgetType::kWindow;
  params.width = kDefaultWindowWidth;
  params.height = kDefaultWindowHeight;

  if (fullscreen) {
    params.width = kDefaultFullscreenWidth;
    params.height = kDefaultFullscreenHeight;
    params.show_state = neva_app_runtime::WebAppWindowBase::CreateParams::
        WindowShowState::kFullscreen;

    // FIXME: fullscreen doesn't imply frameless. This only affects PC thus
    // after removing need to correct related TCs in case of regression
    frameless = true;
  }

  params.type = frameless
              ? neva_app_runtime::WebAppWindowBase::CreateParams::
                  WidgetType::kWindowFrameless
              : neva_app_runtime::WebAppWindowBase::CreateParams::
                  WidgetType::kWindow;

  std::unique_ptr<neva_app_runtime::WebViewProfile> profile;
  if (!profile_name.empty())
    profile = std::make_unique<neva_app_runtime::WebViewProfile>(profile_name);

  auto window = std::make_unique<WamDemoWindow>(params, this);
  auto webview = std::make_unique<WamDemoWebView>(
      system_appid, params.width, params.height, this, profile.get());

  webview->SetAppId(system_appid);
  window->SetWindowProperty("appId", system_appid);
  webview->SetSupportsMultipleWindows(false);

  webview->LoadUrl(appurl.c_str());
  window->AttachWebContents(webview->GetWebContents());
  if (!base::CommandLine::ForCurrentProcess()->HasSwitch(switches::kHidden)) {
    window->Show();
  } else if (IsWayland()) {
    window->Minimize();
  }

#if BUILDFLAG(IS_WEBOS)
  std::string media_codec_capability = GetMediaCodecCapability();
  if (!media_codec_capability.empty())
    webview->SetMediaCodecCapability(media_codec_capability);
#endif  // BUILDFLAG(IS_WEBOS)

  if (base::CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kRemoteDebuggingPort)) {
    webview->SetInspectable(true);
    webview->EnableInspectablePage();
  }

  WamDemoApplication::CreateParams app_params;
  app_params.system_appid = std::move(system_appid);
  app_params.service_appid = appid;
  app_params.url = appurl;
  app_params.window = std::move(window);
  app_params.webview = std::move(webview);
  app_params.profile = std::move(profile);
  appslist_.emplace_back(std::move(app_params), *(system_delegate_.get()));
}

void WamDemoManager::LaunchApplication(const AppLaunchParams& launch_params) {
  std::string system_appid = GetSystemAppId(launch_params.appid);
  neva_app_runtime::WebAppWindowBase::CreateParams params;
  params.web_contents = nullptr;

  if (launch_params.fullscreen) {
    params.width = kDefaultFullscreenWidth;
    params.height = kDefaultFullscreenHeight;
    params.show_state =
        neva_app_runtime::WebAppWindowBase::
            CreateParams::WindowShowState::kFullscreen;
  } else {
    params.pos_x = launch_params.layout_x;
    params.pos_y = launch_params.layout_y;
    params.width = launch_params.layout_width;
    params.height = launch_params.layout_height;
#if BUILDFLAG(IS_WEBOS)
    if (params.pos_x != 0 || params.pos_y != 0) {
      LOG(ERROR) << __func__
                 << " webOS doesn't support setting app's origin to ("
                 << params.pos_x << "," << params.pos_y << "). So reset origin to (0, 0)";
      params.pos_x = 0;
      params.pos_y = 0;
    }
#endif  // BUILDFLAG(IS_WEBOS)
  }

  params.type = launch_params.frameless
              ? neva_app_runtime::WebAppWindowBase::CreateParams::
                    WidgetType::kWindowFrameless
              : neva_app_runtime::WebAppWindowBase::CreateParams::
                    WidgetType::kWindow;

  std::unique_ptr<neva_app_runtime::WebViewProfile> profile;
  if (!launch_params.profile_name.empty()) {
    profile = std::make_unique<neva_app_runtime::WebViewProfile>(
        launch_params.profile_name);
  }

  auto window = std::make_unique<WamDemoWindow>(params, this);
  auto webview = std::make_unique<WamDemoWebView>(system_appid, params.width,
                                                  params.height, this,
                                                  profile.get());
  webview->CreateRenderView();

  if ((launch_params.viewport_width > 0) &&
      (launch_params.viewport_height > 0)) {
    webview->SetViewportSize(
        launch_params.viewport_width, launch_params.viewport_height);
  }

  for (const auto& injection : launch_params.injections)
    webview->RequestInjectionLoading(injection);

  webview->SetAppId(system_appid);
  window->SetWindowProperty("appId", system_appid);
  webview->SetSecurityOrigin(system_appid);
  webview->SetTransparentBackground(launch_params.transparent_background);
  webview->SetSupportsMultipleWindows(false);

  if (!launch_params.group.empty()) {
    std::string system_group = GetSystemGroupName(launch_params.group);
    if (launch_params.group_owner) {
      LOG(INFO) << __func__ << "(): create group " << system_group;
      neva_app_runtime::WindowGroupLayerConfiguration layer_config;
      layer_config.name = kDefaultLayerName;
      layer_config.z_order = 100;

      neva_app_runtime::WindowGroupConfiguration config;
      config.name = system_group;
      config.layers.push_back(layer_config);
      window->CreateGroup(config);
    } else {
      LOG(INFO) << __func__ << "(): attach to group " << system_group;
      webview->SetTransparentBackground(true);
      window->AttachToGroup(system_group, kDefaultLayerName);
    }
  }

  webview->LoadUrl(launch_params.appurl.c_str());
  window->AttachWebContents(webview->GetWebContents());
  window->Show();

#if BUILDFLAG(IS_WEBOS)
  std::string media_codec_capability = GetMediaCodecCapability();
  if (!media_codec_capability.empty())
    webview->SetMediaCodecCapability(media_codec_capability);
#endif  // BUILDFLAG(IS_WEBOS)

  if (base::CommandLine::ForCurrentProcess()->
      GetSwitchValueASCII(switches::kTestType) == "webdriver") {
    webview->EnableInspectablePage();
  }

  WamDemoApplication::CreateParams app_params;
  app_params.system_appid = std::move(system_appid);
  app_params.service_appid = launch_params.appid;
  app_params.url = launch_params.appurl;
  app_params.window = std::move(window);
  app_params.webview = std::move(webview);
  app_params.profile = std::move(profile);
  app_params.launch_params_dict = AppLaunchParams::AsDict(launch_params);
  appslist_.emplace_back(std::move(app_params), *(system_delegate_.get()));
}

WamDemoApplication* WamDemoManager::GetApplicationByServiceAppId(
    const std::string& service_appid) {
  auto app_it = std::find_if(appslist_.begin(), appslist_.end(),
      [&service_appid](WamDemoApplication& app)
      { return app.GetServiceAppId() == service_appid; });
  return (app_it == appslist_.end()) ? nullptr : &(*app_it);
}

WamDemoApplication* WamDemoManager::GetApplicationByWebView(
    const WamDemoWebView* view) {
  auto app_it = std::find_if(appslist_.begin(), appslist_.end(),
      [view](WamDemoApplication& app) { return app.GetWebView() == view; });
  return (app_it == appslist_.end()) ? nullptr : &(*app_it);
}

WamDemoApplication* WamDemoManager::GetApplicationByWindow(
    const WamDemoWindow* window) {
  auto app_it = std::find_if(appslist_.begin(), appslist_.end(),
      [window](WamDemoApplication& app) { return app.GetWindow() == window; });
  return (app_it == appslist_.end()) ? nullptr : &(*app_it);
}

void WamDemoManager::NetworkStateChanged(bool connected) {
  neva_app_runtime::AppRuntimeNetworkChangeNotifier* network_change_notifier =
      neva_app_runtime::AppRuntimeNetworkChangeNotifier::GetInstance();
  if (network_change_notifier)
    network_change_notifier->OnNetworkStateChanged(connected);
}

void WamDemoManager::OnWindowClosing(WamDemoWindow* window) {
  auto app_it = std::find_if(appslist_.begin(), appslist_.end(),
      [window](WamDemoApplication& app) { return app.GetWindow() == window; });

  if (app_it == appslist_.end())
    return;

  // Do not remove red underline on wam_emulator_page.html
  // if render process gone before
  if (!app_it->IsRenderGone())
    delegate_->AppWindowClosing(app_it->GetServiceAppId());

  appslist_.erase(app_it);
}

void WamDemoManager::CursorVisibilityChanged(WamDemoWindow* window,
                                             bool visible) {
  auto app_it = std::find_if(appslist_.begin(), appslist_.end(),
      [window](WamDemoApplication& app) { return app.GetWindow() == window; });

  if (app_it != appslist_.end())
    delegate_->CursorVisibilityChanged(app_it->GetServiceAppId(), visible);
}

void WamDemoManager::OnDidLoadingEnd(WamDemoWebView* view,
                                     int64_t loading_end_us) {
  WamDemoApplication* app = GetApplicationByWebView(view);
  if (app) {
    base::TimeDelta delta = base::TimeDelta::FromInternalValue(loading_end_us);
    delegate_->DidLoadingEnd(app->GetServiceAppId(), delta);
  }
}

void WamDemoManager::OnDidFirstPaint(WamDemoWebView* view,
                                     int64_t first_paint_us) {
  WamDemoApplication* app = GetApplicationByWebView(view);
  if (app) {
    base::TimeDelta delta = base::TimeDelta::FromInternalValue(first_paint_us);
    delegate_->DidFirstPaint(app->GetServiceAppId(), delta);
  }
}

void WamDemoManager::OnDidFirstContentfulPaint(
    WamDemoWebView* view,
    int64_t first_contentful_paint_us) {
  WamDemoApplication* app = GetApplicationByWebView(view);
  if (app) {
    base::TimeDelta delta =
        base::TimeDelta::FromInternalValue(first_contentful_paint_us);
    delegate_->DidFirstContentfulPaint(app->GetServiceAppId(), delta);
  }
}

void WamDemoManager::OnDidFirstImagePaint(WamDemoWebView* view,
                                          int64_t first_image_paint_us) {
  WamDemoApplication* app = GetApplicationByWebView(view);
  if (app) {
    base::TimeDelta delta =
        base::TimeDelta::FromInternalValue(first_image_paint_us);
    delegate_->DidFirstImagePaint(app->GetServiceAppId(), delta);
  }
}

void WamDemoManager::OnDidFirstMeaningfulPaint(
    WamDemoWebView* view,
    int64_t first_meaningful_paint_us) {
  WamDemoApplication* app = GetApplicationByWebView(view);
  if (app) {
    base::TimeDelta delta =
        base::TimeDelta::FromInternalValue(first_meaningful_paint_us);
    delegate_->DidFirstMeaningfulPaint(app->GetServiceAppId(), delta);
  }
}

void WamDemoManager::OnDidLargestContentfulPaint(
    WamDemoWebView* view,
    int64_t largest_contentful_paint_us) {
  WamDemoApplication* app = GetApplicationByWebView(view);
  if (app) {
    base::TimeDelta delta =
        base::TimeDelta::FromInternalValue(largest_contentful_paint_us);
    delegate_->DidLargestContentfulPaint(app->GetServiceAppId(), delta);
  }
}

void WamDemoManager::OnTitleChanged(WamDemoWebView* view,
                                    const std::string& title) {
  WamDemoApplication* app = GetApplicationByWebView(view);
  if (app)
    app->GetWindow()->SetWindowTitle(title);
}

void WamDemoManager::OnRenderProcessGone(WamDemoWebView* view) {
  if (WamDemoApplication* app = GetApplicationByWebView(view)) {
    delegate_->RenderProcessGone(app->GetServiceAppId());
    app->SetRenderGone(true);
    app->GetWindow()->Close();
  }
}

void WamDemoManager::OnRenderProcessCreated(
    WamDemoWebView* view) {
  WamDemoApplication* app = GetApplicationByWebView(view);
  if (app)
    delegate_->RenderProcessCreated(app->GetServiceAppId());
}

void WamDemoManager::OnDocumentLoadFinished(
    WamDemoWebView* view) {
  WamDemoApplication* app = GetApplicationByWebView(view);
  if (app) {
    delegate_->DocumentLoadFinished(app->GetServiceAppId(),
                                    app->GetWebView()->RenderProcessPid(),
                                    app->GetWebView()->GetZoomFactor());
  }
}

void WamDemoManager::OnLoadFailed(WamDemoWebView* view) {
  WamDemoApplication* app = GetApplicationByWebView(view);
  if (app)
    delegate_->LoadFailed(app->GetServiceAppId());
}

void WamDemoManager::OnBrowserControlCommand(
    WamDemoWebView* view,
    const std::string& name,
    const std::vector<std::string>& args) {
  WamDemoApplication* app = GetApplicationByWebView(view);
  if (!app)
    return;

  std::ignore = app->HandleCommand(name, args);
  delegate_->BrowserControlCommandNotify(name, args);
}

std::string WamDemoManager::OnBrowserControlFunction(
    WamDemoWebView* view,
    const std::string& name,
    const std::vector<std::string>& args) {
  std::string result;
  WamDemoApplication* app = GetApplicationByWebView(view);
  if (!app)
    return result;

  // "browsercontrol" is used for BrowserControl support tests and
  // it waits for the processing of commands in a special way.
  if (app->GetServiceAppId() == "browsercontrol") {
    if (name == std::string("identifier")) {
      result = app->GetServiceAppId();
    } else {
      std::stringstream ss;
      ss << args.size();
      ss >> result;
    }
  } else {
    result = app->HandleCommand(name, args);
  }

  delegate_->BrowserControlFunctionNotify(name, args, result);
  return result;
}

void WamDemoManager::OnClose(WamDemoWebView* view) {
  if (WamDemoApplication* app = GetApplicationByWebView(view))
    app->GetWindow()->Close();
}

}  // namespace wam_demo
