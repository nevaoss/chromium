// Copyright 2017 LG Electronics, Inc.
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

#include "neva/wam_demo/wam_demo_service.h"

#include <signal.h>

#include <optional>
#include <sstream>

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/strings/string_number_conversions.h"
#include "build/build_config.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/web_contents.h"
#include "neva/app_runtime/public/proxy_settings.h"
#include "neva/app_runtime/webview_profile.h"
#include "neva/emulator/emulator_urls.h"
#include "neva/wam_demo/wam_demo_app_launch_params.h"
#include "neva/wam_demo/wam_demo_application.h"
#include "neva/wam_demo/wam_demo_emulator_commands.h"
#include "neva/wam_demo/wam_demo_service_utils.h"
#include "neva/wam_demo/wam_demo_switches.h"
#include "neva/wam_demo/wam_demo_webview.h"
#include "neva/wam_demo/wam_demo_window.h"
#include "util.h"

#if defined(USE_OZONE)
#include "ui/ozone/public/ozone_platform.h"
#endif  // defined(USE_OZONE)

namespace wam_demo {

WamDemoService::WamDemoService(content::MainFunctionParams parameters)
    : parameters_(std::move(parameters)), manager_(this) {
  if (!base::CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kDisableHTTPRequestEmulator)) {
    // Adding URL
    emulator::EmulatorDataSource* pEmulatorInterface =
        emulator::EmulatorDataSource::GetInstance();
    pEmulatorInterface->AddURLForPolling(
        emulator::kWam_commandSet, this,
        base::SingleThreadTaskRunner::GetCurrentDefault());
  }
}

void WamDemoService::LaunchApplicationFromCLI(const std::string& appid,
                                              const std::string& appurl,
                                              bool fullscreen,
                                              bool frameless) {
#if BUILDFLAG(IS_WEBOS)
  if (!frameless) {
    LOG(INFO) << __func__ << "(): webOS doesn't use framed windows."
              << " So init window as frameless forcibly";
    frameless = true;
  }
#endif  // BUILDFLAG(IS_WEBOS)
  manager_.LaunchApplicationFromCLI(
      appid, appurl, fullscreen, frameless, profile_name_);
}

void WamDemoService::HandleLaunchApplicationCommand(const std::string& value,
                                                    const std::string& appid,
                                                    const std::string& appurl) {
  AppLaunchParams launch_params;
  launch_params.appid = appid;
  launch_params.appurl = appurl;

  if (!UnpackBool(value, argument::kFullScreen, launch_params.fullscreen))
    LOG(INFO) << __func__ << "(): no valid \'" << argument::kFullScreen << "\'";

  if (!launch_params.fullscreen) {
    UnpackLayoutParams(value,
                       launch_params.layout_x, launch_params.layout_y,
                       launch_params.layout_width, launch_params.layout_height);
#if BUILDFLAG(IS_WEBOS)
    if (launch_params.layout_x != 0 || launch_params.layout_y != 0) {
      LOG(WARNING) << __func__
                 << " webOS doesn't support setting app's origin to ("
                 << launch_params.layout_x << "," << launch_params.layout_y
                 << "). So reset origin to (0, 0)";
      launch_params.layout_x = 0;
      launch_params.layout_y = 0;
    }
#endif  // BUILDFLAG(IS_WEBOS)
  }

  if (!UnpackBool(value, argument::kFramelessWindow, launch_params.frameless)) {
    LOG(INFO) << __func__ << "(): no valid \'"
              << argument::kFramelessWindow << "\'";
  }

#if BUILDFLAG(IS_WEBOS)
  if (!launch_params.frameless) {
    LOG(INFO) << __func__ << "(): webOS doesn't use framed windows."
              << " So init window as frameless forcibly";
    launch_params.frameless = true;
  }
#endif  // BUILDFLAG(IS_WEBOS)

  launch_params.profile_name = profile_name_;

  if (!UnpackViewportParams(value,
                            launch_params.viewport_width,
                            launch_params.viewport_height))
    LOG(INFO) << __func__ << "(): Invalid viewport parameters";

  if (!UnpackBool(value,
                  argument::kTransparentBackground,
                  launch_params.transparent_background)) {
    LOG(INFO) << __func__ << "(): no valid \'"
              << argument::kTransparentBackground << "\'";
  }

  UnpackInjections(value, launch_params.injections);

  if (UnpackString(value, argument::kGroupName, launch_params.group)) {
    if (!UnpackBool(value, argument::kIsOwner, launch_params.group_owner))
      LOG(INFO) << __func__ << "(): Invalid group parameters";
  }

  manager_.LaunchApplication(launch_params);
}

void WamDemoService::EmulatorSendData(const std::string& command,
                                      const std::string& id) {
  emulator::RequestArgs args_vector = {
    {"arg1", &command},
    {"arg2", &id}
  };

  std::string params =
      emulator::EmulatorDataSource::PrepareRequestParams(args_vector);
  emulator::EmulatorDataSource::SetExpectationAsync(
      emulator::kWam_callFunc, params);
}

void WamDemoService::PerformanceEvent(base::TimeDelta delta,
                                      const std::string& appid,
                                      const std::string& type) {
  LOG(INFO) << "Application " << appid << " " << type
            << " is " << delta.InMilliseconds() << " (ms)";
  std::stringstream data_str;
  data_str << type << ":" << base::NumberToString(delta.InMilliseconds());
  EmulatorSendData(data_str.str(), appid);
}

void WamDemoService::AppWindowClosing(const std::string& appid) {
  EmulatorSendData(response::kAppClosed, appid);
}

void WamDemoService::CursorVisibilityChanged(const std::string& appid,
                                             bool shown) {
  std::stringstream data_str;
  data_str << response::kCursorUpdated << ":" << (shown ? "visible" : "hidden");
  EmulatorSendData(data_str.str(), appid);
}

void WamDemoService::DidLoadingEnd(const std::string& appid,
                                   base::TimeDelta delta) {
  PerformanceEvent(delta, appid, response::kLoadingEndTime);
}

void WamDemoService::DidFirstPaint(const std::string& appid,
                                   base::TimeDelta delta) {
  PerformanceEvent(delta, appid, response::kFirstPaintTime);
}

void WamDemoService::DidFirstContentfulPaint(const std::string& appid,
                                             base::TimeDelta delta) {
  PerformanceEvent(delta, appid, response::kFirstContentfulPaintTime);
}

void WamDemoService::DidFirstImagePaint(const std::string& appid,
                                base::TimeDelta delta) {
  PerformanceEvent(delta, appid, response::kFirstImagePaintTime);
}

void WamDemoService::DidFirstMeaningfulPaint(const std::string& appid,
                                      base::TimeDelta delta) {
  PerformanceEvent(delta, appid, response::kFirstMeaningfulPaintTime);
}

void WamDemoService::DidLargestContentfulPaint(const std::string& appid,
                                               base::TimeDelta delta) {
  PerformanceEvent(delta, appid, response::kLargestContentfulPaintTime);
}

void WamDemoService::RenderProcessGone(const std::string& appid) {
  EmulatorSendData(response::kProcessGone, appid);
}

void WamDemoService::RenderProcessCreated(const std::string& appid) {
  EmulatorSendData(response::kAppStarted, appid);
}

void WamDemoService::DocumentLoadFinished(const std::string& appid,
                                          std::optional<pid_t> pid,
                                          float zoom) {
  {
    std::stringstream data_str;
    data_str << response::kZoomUpdated << ":"
             << base::NumberToString((int)std::floor(zoom * 100.f + 0.5));
    EmulatorSendData(data_str.str(), appid);
  }

  if (pid) {
    std::stringstream data_str;
    data_str << response::kPidUpdated << ":"
             << base::NumberToString(pid.value());
    EmulatorSendData(data_str.str(), appid);
  }

  EmulatorSendData(response::kLoadFinished, appid);
}

void WamDemoService::LoadFailed(const std::string& appid) {
  EmulatorSendData(response::kLoadFailed, appid);
}

void WamDemoService::BrowserControlCommandNotify(
    const std::string& name,
    const std::vector<std::string>& args) {
  emulator::RequestArgs args_vector = {
    {"name", &name},
  };

  std::vector<std::string> names;
  names.reserve(args.size());
  for (std::size_t i = 0; i < args.size(); ++i) {
    std::stringstream name_builder;
    name_builder << "arg" << (i+1);
    names.push_back(name_builder.str());
    args_vector.push_back({names.back().c_str(), &args[i]});
  }

  const std::string params =
      emulator::EmulatorDataSource::PrepareRequestParams(args_vector);
  emulator::EmulatorDataSource::SetExpectationAsync(
      emulator::kBrowserControl_sendCommand, params);
}

void WamDemoService::BrowserControlFunctionNotify(
    const std::string& name,
    const std::vector<std::string>& args,
    const std::string& result) {
  emulator::RequestArgs args_vector = {
    {"name", &name},
    {"result", &result},
  };

  std::vector<std::string> names;
  names.reserve(args.size());
  for (std::size_t i = 0; i < args.size(); ++i) {
    std::stringstream name_builder;
    name_builder << "arg" << (i+1);
    names.push_back(name_builder.str());
    args_vector.push_back({names.back().c_str(), &args[i]});
  }

  const std::string params =
      emulator::EmulatorDataSource::PrepareRequestParams(args_vector);
  emulator::EmulatorDataSource::SetExpectationAsync(
      emulator::kBrowserControl_callFunction, params);
}

void WamDemoService::DataUpdated(const std::string& url,
                                 const std::string& value) {
  if (url.compare(emulator::kWam_commandSet) != 0)
    return;

  LOG(INFO) << __func__ << "(): Command is delivered: " << value.c_str();
  std::string appid;
  std::string cmd;
  std::string appurl;

  if (!UnpackGeneralParams(value, appid, cmd, appurl))
    return;

  std::string system_appid = GetSystemAppId(appid);
  WamDemoApplication* app = manager_.GetApplicationByServiceAppId(appid);

  if (cmd == command::kSetProfile) {
    std::string profile_name;
    if (UnpackString(value, argument::kProfile, profile_name)) {
      LOG(INFO) << __func__ << "(): Set profile to = " << profile_name;
      profile_name_ = profile_name;
    } else {
      LOG(INFO) << __func__ << "(): Invalid params";
    }
    return;
  }

  if (app) {
    if (cmd == command::kChangeUrl) {
      app->ChangeUrl(appurl);
      EmulatorSendData(response::kAppStarted, appid);
    } else if (cmd == command::kReloadPage) {
      app->GetWebView()->Reload();
    } else if (cmd == command::kReplaceBaseURL) {
      app->ReplaceBaseUrl(appurl);
    } else if (cmd == command::kStopLoading) {
      app->GetWebView()->StopLoading();
    } else if (cmd == command::kLaunchApp) {
      LOG(INFO) << __func__ << "(): Application already started";
    }
    else if (cmd == command::kStopApp)
      app->GetWindow()->Close();
    else if (cmd == command::kKillApp) {
      const int pid = app->GetWebView()->RenderProcessPid();
      if (pid)
        kill(pid, SIGKILL);
      else
        LOG(INFO) << __func__ << "(): Invalid pid to kill";
    } else if (cmd == command::kUpdateZoom) {
      unsigned zoom;
      if (UnpackUInt(value, argument::kZoomFactor, zoom))
        app->GetWebView()->SetZoomFactor((float)zoom / 100.f);
      else
        LOG(INFO) << __func__ << "(): no valid \'" << argument::kZoomFactor << "\'";

      const std::string zoom_str = base::NumberToString(
          (int)std::floor(app->GetWebView()->GetZoomFactor() * 100.f + 0.5));
      if (!zoom_str.empty()) {
        std::stringstream data_str;
        data_str << response::kZoomUpdated << ":" << zoom_str;
        EmulatorSendData(data_str.str(), appid);
      } else {
        LOG(INFO) << __func__ << "(): Zoom factor is empty";
      }
    } else if (cmd == command::kUpdateAppWindow) {
      int x = 0;
      int y = 0;
      int w = 0;
      int h = 0;
      if (UnpackLayoutParams(value, x, y, w, h))
        app->GetWindow()->SetBounds(x, y, w, h);
      else
        LOG(INFO) << __func__ << "(): Invalid layout parameters";
    } else if (cmd == command::kMinimizeApp) {
      app->GetWindow()->Minimize();
    } else if (cmd == command::kIsKeyboardVisible) {
      std::stringstream data_str;
      data_str << response::kKeyboardVisibility << ":"
               << (app->GetWindow()->IsKeyboardVisible() ? "yes" : "no");
      EmulatorSendData(data_str.str(), appid);
    } else if (cmd == command::kSetWindowProperty) {
      std::string window_property_name;
      std::string window_property_value;
      if (UnpackString(value, argument::kName, window_property_name) &&
          UnpackString(value, argument::kValue, window_property_value)) {
        app->GetWindow()->SetWindowProperty(window_property_name,
                                            window_property_value);
      } else
        LOG(INFO) << __func__ << "(): Invalid window property";
    } else if (cmd == command::kSetBackgroundColor) {
      int r = 0;
      int g = 0;
      int b = 0;
      if (UnpackBackgroundColor(value, r, g, b))
        app->GetWebView()->SetBackgroundColor(r, g, b,
                                            argument::kDefaultAlphaValue);
    } else if (cmd == command::kSetBoardType) {
      std::string board_type;
      if (UnpackString(value, argument::kBoardType, board_type)) {
        LOG(INFO) << __func__ << "(): board_type: " << board_type;
        app->GetWebView()->SetBoardType(board_type);
      } else
        LOG(INFO) << __func__ << "(): Invalid board type";
    } else if (cmd == command::kLoadInjections) {
      std::vector<std::string> injections;
      if (UnpackInjections(value, injections)) {
        for (const auto& injection : injections)
          app->GetWebView()->RequestInjectionLoading(injection);
      }
    } else if (cmd == command::kClearInjections) {
      app->GetWebView()->RequestClearInjections();
    } else if (cmd == command::kDecidePolicyForResponse) {
      app->GetWebView()->SetDecidePolicyForResponse();
    } else if (cmd == command::kDeleteWebStorages) {
      app->GetWebView()->DeleteWebStorages(system_appid);
    } else if (cmd == command::kClearBrowsingData) {
      unsigned browsing_data_mask = 0;
      if (UnpackUInt(value, argument::kBrowsingDataMask, browsing_data_mask)) {
        LOG(INFO) << __func__
                  << "(): browsing_data_mask: " << browsing_data_mask;
        app->GetWebView()->GetProfile()->RemoveBrowsingData(browsing_data_mask);
      } else {
        LOG(INFO) << __func__ << "(): Invalid browsing data mask";
      }
    } else if (cmd == command::kGoBack) {
      app->GetWebView()->GoBack();
    } else if (cmd == command::kCanGoBack) {
      std::stringstream data_str;
      data_str << response::kCanGoBackAbility << ":"
               << (app->GetWebView()->CanGoBack() ? "yes" : "no");
      EmulatorSendData(data_str.str(), appid);
    } else if (cmd == command::kGetDocumentTitle) {
      std::stringstream data_str;
      data_str << response::kDocumentTitle << ":"
               << app->GetWebView()->DocumentTitle();
      EmulatorSendData(data_str.str(), appid);
    } else if (cmd == command::kGetDocumentUrl) {
      std::stringstream data_str;
      data_str << response::kDocumentUrl << ":" << app->GetWebView()->GetUrl();
      EmulatorSendData(data_str.str(), appid);
    } else if (cmd == command::kDoNotTrack) {
      bool dnt;
      if (UnpackBool(value, argument::kEnable, dnt))
        app->GetWebView()->SetDoNotTrack(dnt);
      else
        LOG(INFO) << __func__ << "(): no valid \'" << argument::kEnable << "\'";
    } else if (cmd == command::kDisableBackButton) {
      bool disable;
      if (UnpackBool(value, argument::kDisable, disable))
        app->GetWebView()->SetBackHistoryKeyDisabled(disable);
      else
        LOG(INFO) << __func__ << "(): no valid \'" << argument::kDisable << "\'";
    } else if (cmd == command::kSimulateNetworkState) {
      bool connected;
      if (UnpackBool(value, argument::kConnected, connected)) {
        manager_.NetworkStateChanged(connected);
        if (connected) {
          app->GetWebView()->LoadFailedUrlIfNeccessary();
        }
      } else {
        LOG(INFO) << __func__ << "(): no valid \'" <<
                     argument::kConnected << "\'";
      }
    } else if (cmd == command::kIgnoreSSLError) {
      app->GetWebView()->SetSSLCertErrorPolicy(
          neva_app_runtime::SSL_CERT_ERROR_POLICY_IGNORE);
    } else if (cmd == command::kResetCompositorPainting) {
      app->GetWindow()->RecreatedWebContents();
    } else if (cmd == command::kAllowXFrameSameOrigin) {
      app->GetWebView()->SetXFrameOptionsCrossOriginAllowed(true);
    } else if (cmd == command::kSetHTMLSystemKeyboardEnabled) {
      app->GetWebView()->SetEnableHtmlSystemKeyboardAttr(true);
    } else if (cmd == command::kSetMediaCodecCapability) {
      std::string media_codec_capability;
      if (UnpackString(value, argument::kMediaCodecCapability, media_codec_capability)) {
        LOG(INFO) << __func__ << "(): media_codec_capability: " << media_codec_capability;
        app->GetWebView()->SetMediaCodecCapability(media_codec_capability);
      } else
        LOG(INFO) << __func__ << "(): Invalid media codec capability";
    } else if (cmd == command::kAllowUniversalAccessFromFileUrls) {
      app->GetWebView()->SetAllowUniversalAccessFromFileUrls(true);
    } else if (cmd == command::kAllowLocalResourceLoad) {
      app->GetWebView()->SetAllowLocalResourceLoad(true);
    } else if (cmd == command::kDisableWebSecurity) {
      app->GetWebView()->SetWebSecurityEnabled(false);
    } else if (cmd == command::kResumeDOM) {
      app->GetWebView()->ResumeWebPageDOM();
    } else if (cmd == command::kSuspendDOM) {
      app->GetWebView()->SuspendWebPageDOM();
    } else if (cmd == command::kResumeMedia) {
      app->GetWebView()->ResumeWebPageMedia();
    } else if (cmd == command::kSuspendPainting) {
      app->GetWebView()->SuspendPaintingAndSetVisibilityHidden();
    } else if (cmd == command::kResumePainting) {
      app->GetWebView()->ResumePaintingAndSetVisibilityVisible();
    } else if (cmd == command::kSetAcceptLanguages) {
      std::string language("ko");
      if (UnpackString(value, argument::kAcceptLanguages, language)) {
        LOG(INFO) << __func__ << "(): Set accept languages to " << language;
        app->GetWebView()->SetAcceptLanguages(language);
        app->GetWebView()->Reload();
      } else {
        LOG(INFO) << __func__ << "(): Invalid params";
      }
    } else if (cmd == command::kRunJavaScript) {
      std::string jscode;
      if (UnpackString(value, argument::kJSCode, jscode))
        app->GetWebView()->RunJavaScript(jscode);
    } else if (cmd == command::kRunJSInAllFrames) {
      std::string jscode;
      if (UnpackString(value, argument::kJSCode, jscode))
        app->GetWebView()->RunJavaScriptInAllFrames(jscode);
    } else if (cmd == command::kSuspendMedia) {
      app->GetWebView()->SuspendWebPageMedia();
    } else if (cmd == command::kSetProxyServer) {
      neva_app_runtime::ProxySettings proxy_settings;
      proxy_settings.enabled = true;
      if (UnpackBool(value, argument::kProxyEnabled, proxy_settings.enabled)) {
        LOG(INFO) << __func__ << "(): proxy "
                  << (proxy_settings.enabled ? "enabled" : "disabled");
        if (proxy_settings.enabled &&
            UnpackString(value, argument::kProxyServer, proxy_settings.ip) &&
            UnpackString(value, argument::kProxyPort, proxy_settings.port)) {
          LOG(INFO) << __func__
                    << "(): Change proxy server to = " << proxy_settings.ip
                    << ":" << proxy_settings.port;
          if (UnpackString(value, argument::kProxyLogin,
                           proxy_settings.username) &&
              UnpackString(value, argument::kProxyPassword,
                           proxy_settings.password)) {
            LOG(INFO) << __func__
                      << "(): Proxy server login: " << proxy_settings.username
                      << ", password: " << proxy_settings.password;
          }
          if (UnpackString(value, argument::kProxyBypassList,
                           proxy_settings.bypass_list)) {
            LOG(INFO) << __func__ << "(): Proxy server bypass list: "
                      << proxy_settings.bypass_list;
          }
        }
        app->GetWebView()->GetProfile()->SetProxyServer(proxy_settings);
        app->GetWebView()->LoadUrl(app->GetUrl());
      } else
        LOG(INFO) << __func__ << "(): Invalid params";
    } else if (cmd == command::kSetExtraWebSocketHeader) {
      std::string header;
      std::string val;
      if (UnpackString(value, argument::kHeaderName, header) &&
          UnpackString(value, argument::kHeaderValue, val)) {
        LOG(INFO) << __func__ << "(): Added extra WebSocket header = "
            << header << ":" << val;
        app->GetWebView()->GetProfile()->AppendExtraWebSocketHeader(header,
                                                                    val);
      } else {
        LOG(INFO) << __func__ << "(): Invalid params";
      }
    } else if (cmd == command::kIsProfileCreated) {
      base::FilePath path =
          app->GetWebView()->GetWebContents()->GetBrowserContext()->GetPath();
      std::stringstream data_str;
      data_str << response::kProfileCreated << ":"
               << base::MakeAbsoluteFilePath(path).value() << ":"
               << (base::PathExists(path) ? "yes" : "no");
      EmulatorSendData(data_str.str(), appid);
    } else if (cmd == command::kFlushCookies) {
      app->GetWebView()->GetProfile()->FlushCookieStore();
    } else if (cmd == command::kGetUserAgent) {
      const std::string user_agent = app->GetWebView()->UserAgent();
      std::stringstream data_str;
      data_str << response::kUserAgentIs << ":"
               << app->GetWebView()->UserAgent();
      EmulatorSendData(data_str.str(), appid);
    } else if (cmd == command::kSetUserAgent) {
      std::string user_agent;
      if (UnpackString(value, argument::kUserAgent, user_agent)) {
        LOG(INFO) << __func__ << "(): user_agent: " << user_agent;
        app->GetWebView()->SetUserAgent(user_agent);
      } else
        LOG(INFO) << __func__ << "(): Invalid user agent";
    } else if (cmd == command::kGetPid) {
      const std::string pid =
          base::NumberToString(app->GetWebView()->RenderProcessPid());
      if (!pid.empty()) {
        std::stringstream data_str;
        data_str << response::kPidRequested << ":" << pid;
        EmulatorSendData(data_str.str(), appid);
      }
      else
        LOG(INFO) << __func__ << "(): Invalid process ID";
    } else if (cmd == command::kSetFocus) {
      bool set = false;
      if (UnpackBool(value, argument::kSet, set))
        app->GetWebView()->SetFocus(set);
    } else if (cmd == command::kSetFontFamily) {
      std::string font_family;
      if (UnpackString(value, argument::kFontFamily, font_family))
        app->GetWebView()->SetStandardFontFamily(font_family);
      else
        LOG(INFO) << __func__ << "(): Invalid font family";
    } else if (cmd == command::kSetUseVirtualKeyboard) {
      bool enable = false;
      if (!UnpackBool(value, argument::kEnable, enable)) {
        LOG(INFO) << __func__ << "(): no valid \'"
                  << argument::kEnable << "\'";
      } else
        app->GetWindow()->SetUseVirtualKeyboard(enable);
    } else if (cmd == command::kResizeWindow) {
      int width = 0, height = 0;
      if (UnpackWindowSize(value, width, height))
        app->GetWindow()->Resize(width, height);
    } else if (cmd == command::kXInputActivate) {
      app->GetWindow()->XInputActivate();
    } else if (cmd == command::kXInputDeactivate) {
      app->GetWindow()->XInputDeactivate();
    } else if (cmd == command::kXInputInvokeAction) {
      std::string keysym_str;
      if (UnpackString(value, argument::kKeySym, keysym_str)) {
        int keysym;
        if (base::StringToInt(keysym_str, &keysym))
          app->GetWindow()->XInputInvokeAction((uint32_t)keysym);
      }
    } else if (cmd == command::kSetAllowFakeBoldText) {
      bool allow = false;
      if (!UnpackBool(value, argument::kAllow, allow))
        LOG(INFO) << __func__ << "(): no valid \'" << argument::kAllow << "\'";
      else
        app->GetWebView()->SetAllowFakeBoldText(allow);
    } else if (cmd == command::kSetDisallowScrollbarsInMainFrame) {
      bool disallow = false;
      if (!UnpackBool(value, argument::kDisallow, disallow))
        LOG(INFO) << __func__ << "(): no valid \'" << argument::kDisallow << "\'";
      else
        app->GetWebView()->SetDisallowScrollbarsInMainFrame(disallow);
    } else if (cmd == command::kSetOpacity) {
      float opacity = 1.f;
      if (UnpackFloat(value, argument::kOpacity, opacity))
        app->GetWindow()->SetOpacity(opacity);
    } else if (cmd == command::kSetWindowState) {
      neva_app_runtime::WidgetState state =
          neva_app_runtime::WidgetState::UNINITIALIZED;
      if (UnpackWindowState(value, state)) {
        LOG(INFO) << __func__ << "(): Set webOS window host state: "
                  << GetWidgetStateString(state);
        if (state == neva_app_runtime::WidgetState::FULLSCREEN) {
          auto* web_contents = app->GetWebView()->GetWebContents();
          if (web_contents->WasDiscarded()) {
            LOG(INFO) << __func__
                      << "(): Reload the page since it was discarded.";
            app->GetWindow()->AttachWebContents(web_contents);
            app->GetWebView()->SetVisibilityState(
                neva_app_runtime::WebPageVisibilityState::
                    WebPageVisibilityStateVisible);
          }
        }
        app->GetWindow()->SetWindowHostState(state);
      }
    } else if (cmd == command::kGetWindowState) {
      const neva_app_runtime::WidgetState new_state =
          app->GetWindow()->GetWindowHostState();
      std::stringstream data_str;
      data_str << response::kWindowStateRequested << ":"
               << GetWidgetStateString(new_state);
      EmulatorSendData(data_str.str(), appid);
    } else if (cmd == command::kGetWindowStateAboutToChange) {
      const neva_app_runtime::WidgetState state =
          app->GetWindow()->GetWindowHostStateAboutToChange();
      std::stringstream data_str;
      data_str << response::kWindowStateAboutToChangeRequested << ":"
               << GetWidgetStateString(state);
      EmulatorSendData(data_str.str(), appid);
    } else if (cmd == command::kNotifyMemoryPressure) {
      std::string level;
      if (UnpackString(value, argument::kMemoryPressureLevel, level)) {
        WamDemoWebView::MemoryPressureLevel converted_level;
        if (level == "none") {
          converted_level = WamDemoWebView::MEMORY_PRESSURE_NONE;
        } else if (level == "low") {
          converted_level = WamDemoWebView::MEMORY_PRESSURE_LOW;
        } else if (level == "critical") {
          converted_level = WamDemoWebView::MEMORY_PRESSURE_CRITICAL;
        } else {
          LOG(ERROR) << __func__ << "(): invalid pressure level: " << level;
          return;
        }

        LOG(INFO) << __func__ << "(): memory pressure level: " << level;
        app->GetWebView()->NotifyMemoryPressure(converted_level);
      } else {
        LOG(ERROR) << __func__ << "(): Invalid memory pressure param.";
        return;
      }
    } else if (cmd == command::kAddUserStyleSheet) {
      std::string user_stylesheet;
      if (UnpackString(value, argument::kUserStyleSheet, user_stylesheet)) {
        LOG(INFO) << __func__ << "(): user_stylesheet: " << user_stylesheet;
        app->GetWebView()->AddUserStyleSheet(user_stylesheet);
      } else {
        LOG(INFO) << __func__ << "(): Invalid user stylesheet type";
      }
    } else if (cmd == command::kSetCustomCursor) {
      auto argv0(parameters_.command_line->GetProgram());
      auto image_path(argv0.DirName().Append("cursor.png"));
      app->GetWindow()->SetCustomCursor(
          neva_app_runtime::CustomCursorType::kPath, image_path.value().c_str(),
          0, 0);
    } else if (cmd == command::kSetVisibilityState) {
      std::string visibility_state;
      if (UnpackString(value, argument::kVisibilityState, visibility_state)) {
        if (visibility_state.compare("Visible") == 0) {
          app->GetWebView()->SetVisibilityState(
              neva_app_runtime::WebPageVisibilityState::
                  WebPageVisibilityStateVisible);
        } else if (visibility_state.compare("Hidden") == 0) {
          app->GetWebView()->SetVisibilityState(
              neva_app_runtime::WebPageVisibilityState::
                  WebPageVisibilityStateHidden);
        } else if (visibility_state.compare("Launching") == 0) {
          app->GetWebView()->SetVisibilityState(
              neva_app_runtime::WebPageVisibilityState::
                  WebPageVisibilityStateLaunching);
        }
      }
      else
        LOG(INFO) << __func__ << "(): Invalid visibility state";
    } else if (cmd == command::kSetGroupKeyMask) {
      unsigned key_mask = 0;
      if (UnpackUInt(value, argument::kKeyMask, key_mask)) {
        app->GetWindow()->SetGroupKeyMask(
            static_cast<neva_app_runtime::KeyMask>(key_mask));
      }
    } else if (cmd == command::kSetKeyMask) {
      unsigned key_mask = 0;
      bool set = false;
      if (UnpackUInt(value, argument::kKeyMask, key_mask) &&
          UnpackBool(value, argument::kSet, set))
        app->GetWindow()->SetKeyMask(
            static_cast<neva_app_runtime::KeyMask>(key_mask), set);
    } else if (cmd == command::kEnableInspectablePage) {
      app->GetWebView()->EnableInspectablePage();
    } else if (cmd == command::kDisableInspectablePage) {
      app->GetWebView()->DisableInspectablePage();
    } else if (cmd == command::kSetInspectable) {
      bool enable;
      if (UnpackBool(value, argument::kEnable, enable))
        app->GetWebView()->SetInspectable(enable);
      else
        LOG(INFO) << __func__ << "(): no valid \'" << argument::kEnable << "\'";
    } else if (cmd == command::kGetDevToolsEndpoint) {
      char* ip = ::getenv("NEVA_EMULATOR_SERVER_ADDRESS");
      int port = emulator::kEmulatorDefaultPort;
      std::stringstream data_str;
      data_str << response::kDevToolsEndpoint << ":"
               << util::get_my_ip_to(ip, port) << ","
               << base::NumberToString(app->GetWebView()->DevToolsPort());
      EmulatorSendData(data_str.str(), appid);
    } else if (cmd == command::kSetInputRegion) {
      neva_app_runtime::WebAppWindowBase::Region input_region;
      int width = app->GetWindow()->GetWidth();
      int height = app->GetWindow()->GetHeight();
      if (UnpackInputRegion(value, argument::kInputRegion, input_region, width,
                            height)) {
        std::vector<neva_app_runtime::WebAppWindowBase::Region> region;
        region.push_back(input_region);
        app->GetWindow()->SetInputRegion(region);
      } else {
        LOG(ERROR) << __func__ << "(): Command \'" << cmd
                   << "\': there is no correct input region";
      }
    } else if (cmd == command::kSetMediaCapturePermission) {
      app->GetWebView()->SetMediaCapturePermission();
    } else if (cmd == command::kClearMediaCapturePermission) {
      app->GetWebView()->ClearMediaCapturePermission();
    } else if (cmd == command::kFocusGroupOwner) {
      app->GetWindow()->FocusGroupOwner();
    } else if (cmd == command::kFocusGroupLayer) {
      app->GetWindow()->FocusGroupLayer();
    } else if (cmd == command::kDetachGroup) {
      app->GetWindow()->DetachGroup();
    } else if (cmd == command::kEnableUnlimitedMediaPolicy) {
      app->GetWebView()->SetUseUnlimitedMediaPolicy(true);
      app->GetWebView()->Reload();
    } else if (cmd == command::kDisableUnlimitedMediaPolicy) {
      app->GetWebView()->SetUseUnlimitedMediaPolicy(false);
      app->GetWebView()->Reload();
    } else if (cmd == command::kGetPageLifecycleState) {
      std::stringstream data_str;
      data_str << response::kPageLifecycleState << ":";
      neva_app_runtime::PageLifecycleState state =
          app->GetWebView()->GetPageLifecycleState();
      switch (state) {
        case neva_app_runtime::PageLifecycleState::Visible:
          data_str << "Visible";
          break;
        case neva_app_runtime::PageLifecycleState::Hidden:
          data_str << "Hidden";
          break;
        case neva_app_runtime::PageLifecycleState::Frozen:
          data_str << "Frozen";
          break;
        case neva_app_runtime::PageLifecycleState::Discarded:
          data_str << "Discarded";
          break;
        default:
          data_str << "Unknown";
      }
      EmulatorSendData(data_str.str(), appid);
    } else {
      LOG(INFO) << __func__ << "(): Command \'" << cmd
                << "\' is not supported in current mode";
    }
  } else {
    if (cmd == command::kLaunchApp) {
      HandleLaunchApplicationCommand(value, appid, appurl);
      EmulatorSendData(response::kAppStarted, appid);
    } else {
      LOG(INFO) << __func__ << "(): Command \'" << cmd
                << "\' is not supported in current mode";
    }
  }
}

}  // namespace wam_demo
