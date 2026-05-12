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

#ifndef NEVA_WAM_DEMO_WAM_DEMO_APPLICATION_H_
#define NEVA_WAM_DEMO_WAM_DEMO_APPLICATION_H_

#include <memory>
#include <string>

#include "base/time/time.h"
#include "base/values.h"
#include "neva/app_runtime/public/app_runtime_constants.h"
#include "neva/pal_service/public/platform_application_delegate.h"

namespace neva_app_runtime {

class WebViewProfile;

}  // namespace neva_app_runtime

namespace pal {

class PlatformSystemHandler;
class PlatformSystemDelegate;
class PlatformManagerDelegate;

}  // namespace pal

namespace wam_demo {

class WamDemoWindow;
class WamDemoWebView;

class WamDemoApplication : public pal::PlatformApplicationDelegate {
 public:
  struct CreateParams {
    CreateParams();
    ~CreateParams();
    CreateParams(CreateParams&&);
    CreateParams& operator=(CreateParams&&);

    std::string system_appid;
    std::string service_appid;
    std::string url;
    std::unique_ptr<WamDemoWindow> window;
    std::unique_ptr<WamDemoWebView> webview;
    std::unique_ptr<neva_app_runtime::WebViewProfile> profile;
    std::unique_ptr<base::Value::Dict> launch_params_dict;
  };

  WamDemoApplication(CreateParams params,
                     pal::PlatformSystemDelegate& system_delegate);

  WamDemoApplication(WamDemoApplication&& other);
  WamDemoApplication& operator = (WamDemoApplication&& other);

  ~WamDemoApplication() override;

  const std::string& GetSystemAppId() const;
  const std::string& GetServiceAppId() const;
  const std::string& GetUrl() const;

  WamDemoWindow* GetWindow() const;
  WamDemoWebView* GetWebView() const;
  neva_app_runtime::WebViewProfile* GetProfile() const;

  bool IsRenderGone() const;
  base::Time GetCreationTime() const;

  void SetRenderGone(bool gone);
  void ChangeUrl(std::string url);
  void ReplaceBaseUrl(const std::string& url);
  std::string HandleCommand(const std::string& name,
                            const std::vector<std::string>& args);

  // pal::PlatofrmApplicationDelegate
  std::string GetIdentifier() const override;
  std::string GetLaunchParams() const override;
  std::string GetFolderPath() const override;
  std::string GetTrustLevel() const override;

 private:
  std::string system_appid_;
  std::string service_appid_;
  std::string url_;
  std::unique_ptr<neva_app_runtime::WebViewProfile> profile_;
  std::unique_ptr<WamDemoWindow> window_;
  std::unique_ptr<WamDemoWebView> webview_;
  std::unique_ptr<base::Value::Dict> launch_params_dict_;
  bool render_gone_ = false;
  base::Time creation_time_ = base::Time::Now();
  std::unique_ptr<pal::PlatformSystemHandler> platform_system_handler_;
};

}  // namespace wam_demo

#endif  // NEVA_WAM_DEMO_WAM_DEMO_APPLICATION_H_
