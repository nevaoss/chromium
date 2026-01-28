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

#ifndef NEVA_WAM_DEMO_WAM_DEMO_WEBVIEW_H_
#define NEVA_WAM_DEMO_WAM_DEMO_WEBVIEW_H_

#include "base/memory/raw_ptr.h"
#include "base/time/time.h"
#include "content/public/common/main_function_params.h"
#include "neva/app_runtime/public/app_runtime_constants.h"
#include "neva/app_runtime/public/webview_base.h"

namespace neva_app_runtime {

class WebViewProfile;

}  // namespace neva_app_runtime

namespace wam_demo {

class WamDemoWebViewObserver;

class WamDemoWebView : public neva_app_runtime::WebViewBase {
 public:
  WamDemoWebView(std::string appid,
                 WamDemoWebViewObserver* observer);

  WamDemoWebView(std::string appid,
            int width, int height,
            WamDemoWebViewObserver* observer,
            neva_app_runtime::WebViewProfile* profile = nullptr);

  WamDemoWebView(const WamDemoWebView&) = delete;
  WamDemoWebView& operator = (const WamDemoWebView&) = delete;
  ~WamDemoWebView() override;

  // from WebViewDelegate
  void OnLoadProgressChanged(double progress) override;
  void DidFirstFrameFocused() override;
  void DidLoadingEnd(int64_t loading_end_us) override;
  void DidFirstPaint(int64_t first_paint_us) override;
  void DidFirstContentfulPaint(int64_t first_contentful_paint_us) override;
  void DidFirstImagePaint(int64_t first_image_paint_us) override;
  void DidFirstMeaningfulPaint(int64_t first_meaningful_paint_us) override;
  void DidLargestContentfulPaint(int64_t largest_contentful_paint_us) override;
  void TitleChanged(const std::string& title) override;
  void NavigationHistoryChanged() override;
  void Close() override;
  bool DecidePolicyForResponse(bool is_main_frame,
                               int status_code,
                               const std::string& url,
                               const std::string& status_text) override;
  bool AcceptsVideoCapture() override;
  bool AcceptsAudioCapture() override;
  void LoadStarted() override;
  void LoadFinished(const std::string& url) override;
  void LoadFailed(const std::string& url,
                  int error_code,
                  const std::string& error_description) override;
  void LoadAborted(const std::string& url) override;
  void LoadStopped() override;
  void RenderProcessCreated(int pid) override;
  void RenderProcessGone() override;
  void DocumentLoadFinished() override;
  void DidStartNavigation(const std::string& url, bool is_main_frame) override;
  void DidFinishNavigation(const std::string& url, bool is_main_frame) override;
  void DidHistoryBackOnTopPage() override;
  void DidClearWindowObject() override;
  void DidSwapCompositorFrame() override;
  void DidDropAllPeerConnections(
      neva_app_runtime::DropPeerConnectionReason reason) override;
  void DidResumeDOM() override;

  // from WebViewControllerDelegate
  void RunCommand(
      const std::string& name,
      const std::vector<std::string>& arguments) override;

  std::string RunFunction(
      const std::string& name,
      const std::vector<std::string>& arguments) override;

  // from WebViewBase
  void Reload();

  // Additional methods for testing
  void SetMediaCapturePermission();
  void ClearMediaCapturePermission();
  void SetDecidePolicyForResponse();
  bool LoadFailedUrlIfNeccessary();

 private:
  void LoadErrorPage(int error_code);

  raw_ptr<WamDemoWebViewObserver> observer_;
  const std::string error_page_base_url_;
  std::string load_failed_url_;
  std::string title_;
  std::string progress_;
  bool accepts_video_capture_ = false;
  bool accepts_audio_capture_ = false;
  std::string appid_;
  static bool policy_;
};

}  // namespace wam_demo

#endif  // NEVA_WAM_DEMO_WAM_DEMO_WEBVIEW_H_
