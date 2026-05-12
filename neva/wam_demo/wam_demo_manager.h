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

#ifndef NEVA_WAM_DEMO_WAM_DEMO_MANAGER_H_
#define NEVA_WAM_DEMO_WAM_DEMO_MANAGER_H_

#include <string>
#include <vector>

#include "base/memory/raw_ptr.h"
#include "neva/wam_demo/wam_demo_app_launch_params.h"
#include "neva/wam_demo/wam_demo_application.h"
#include "neva/wam_demo/wam_demo_webview_observer.h"
#include "neva/wam_demo/wam_demo_window_observer.h"

namespace pal {

class PlatformSystemDelegate;

}  // namespace pal

namespace wam_demo {

class WamDemoManagerDelegate;
class WamDemoWebView;
class WamDemoWindow;

class WamDemoManager : public WamDemoWindowObserver,
                       public WamDemoWebViewObserver {
 public:
  explicit WamDemoManager(WamDemoManagerDelegate* delegate);
  ~WamDemoManager();
  WamDemoManager(const WamDemoManager&) = delete;
  WamDemoManager& operator = (const WamDemoManager&) = delete;

  void LaunchApplicationFromCLI(const std::string& appid,
                                const std::string& appurl,
                                bool fullscreen,
                                bool frameless,
                                const std::string& profile_name);
  void LaunchApplication(const AppLaunchParams& launch_params);

  WamDemoApplication* GetApplicationByServiceAppId(
      const std::string& service_appid);

  WamDemoApplication* GetApplicationByWebView(
      const WamDemoWebView* webview);

  WamDemoApplication* GetApplicationByWindow(
      const WamDemoWindow* window);

  void NetworkStateChanged(bool connected);

  // from WamDemoWindowObserver
  void OnWindowClosing(WamDemoWindow* window) override;
  void CursorVisibilityChanged(WamDemoWindow* window, bool visible) override;

  // from WamDemoWebViewObserver
  void OnDocumentLoadFinished(WamDemoWebView* view) override;
  void OnLoadFailed(WamDemoWebView* view) override;
  void OnRenderProcessGone(WamDemoWebView* view) override;
  void OnRenderProcessCreated(WamDemoWebView* view) override;
  void OnDidLoadingEnd(WamDemoWebView* view, int64_t loading_end_us) override;
  void OnDidFirstPaint(WamDemoWebView* view, int64_t first_paint_us) override;
  void OnDidFirstContentfulPaint(WamDemoWebView* view,
                                 int64_t first_contentful_paint_us) override;
  void OnDidFirstImagePaint(WamDemoWebView* view,
                            int64_t first_image_paint_us) override;
  void OnDidFirstMeaningfulPaint(WamDemoWebView* view,
                                 int64_t first_meaningful_paint_us) override;
  void OnDidLargestContentfulPaint(
      WamDemoWebView* view,
      int64_t largest_contentful_paint_us) override;
  void OnTitleChanged(WamDemoWebView* view, const std::string& title) override;
  void OnBrowserControlCommand(WamDemoWebView* view,
                               const std::string& name,
                               const std::vector<std::string>& args) override;
  std::string OnBrowserControlFunction(
      WamDemoWebView* view,
      const std::string& name,
      const std::vector<std::string>& args) override;
  void OnClose(WamDemoWebView* view) override;

 private:
  std::vector<WamDemoApplication> appslist_;
  std::unique_ptr<pal::PlatformSystemDelegate> system_delegate_;
  raw_ptr<WamDemoManagerDelegate> delegate_;
};

}  // namespace wam_demo

#endif  // NEVA_WAM_DEMO_WAM_DEMO_MANAGER_H_
