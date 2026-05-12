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

#include "neva/wam_demo/wam_demo_application.h"

#include "base/json/json_writer.h"
#include "neva/app_runtime/webview_profile.h"
#include "neva/pal_service/platform_system_handler.h"
#include "neva/wam_demo/wam_demo_webview.h"
#include "neva/wam_demo/wam_demo_window.h"
#include "url/gurl.h"

namespace wam_demo {

WamDemoApplication::CreateParams::CreateParams() = default;

WamDemoApplication::CreateParams::~CreateParams() = default;

WamDemoApplication::CreateParams::CreateParams(CreateParams&&) = default;

WamDemoApplication::CreateParams& WamDemoApplication::CreateParams::operator=(
    CreateParams&&) = default;

WamDemoApplication::WamDemoApplication(
    CreateParams params,
    pal::PlatformSystemDelegate& system_delegate)
    : system_appid_(std::move(params.system_appid)),
      service_appid_(std::move(params.service_appid)),
      url_(std::move(params.url)),
      profile_(std::move(params.profile)),
      window_(std::move(params.window)),
      webview_(std::move(params.webview)),
      launch_params_dict_(std::move(params.launch_params_dict)),
      platform_system_handler_(
          std::make_unique<pal::PlatformSystemHandler>(*this,
                                                       system_delegate)) {}

WamDemoApplication::WamDemoApplication(WamDemoApplication&& other) = default;

WamDemoApplication& WamDemoApplication::operator=(WamDemoApplication&& other) =
    default;

WamDemoApplication::~WamDemoApplication() {
  webview_.reset();
  if (window_)
    window_->DetachWebContents();
}

const std::string& WamDemoApplication::GetSystemAppId() const {
  return system_appid_;
}

const std::string& WamDemoApplication::GetServiceAppId() const {
  return service_appid_;
}

const std::string& WamDemoApplication::GetUrl() const {
  return url_;
}

WamDemoWindow* WamDemoApplication::GetWindow() const {
  return window_.get();
}

WamDemoWebView* WamDemoApplication::GetWebView() const {
  return webview_.get();
}

neva_app_runtime::WebViewProfile* WamDemoApplication::GetProfile() const {
  return profile_.get();
}

bool WamDemoApplication::IsRenderGone() const {
  return render_gone_;
}

base::Time WamDemoApplication::GetCreationTime() const {
  return creation_time_;
}

void WamDemoApplication::SetRenderGone(bool gone) {
  render_gone_ = gone;
}

void WamDemoApplication::ChangeUrl(std::string url) {
  webview_->LoadUrl("about:blank");
  url_ = std::move(url);
  webview_->LoadUrl(url_.c_str());
}

void WamDemoApplication::ReplaceBaseUrl(const std::string& url) {
  webview_->ReplaceBaseURL(url, url_);
  // Base URL is replaced now and we need to reload page
  webview_->RunJavaScript("location.reload();");
}

std::string WamDemoApplication::HandleCommand(
    const std::string& name,
    const std::vector<std::string>& args) {
  return platform_system_handler_->Handle(name, args);
}

std::string WamDemoApplication::GetIdentifier() const {
  return GetSystemAppId();
}

std::string WamDemoApplication::GetLaunchParams() const {
  std::string result;
  if (launch_params_dict_ &&
      base::JSONWriter::Write(*launch_params_dict_, &result))
    return result;
  return std::string("{}");
}

std::string WamDemoApplication::GetFolderPath() const {
  return GURL(url_).path();
}

std::string WamDemoApplication::GetTrustLevel() const {
  // All applications launched by wam_demo are "trusted".
  return std::string("trusted");
}

}  // namespace wam_demo
