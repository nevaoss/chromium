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

#include "neva/wam_demo/wam_demo_webview.h"

#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/strings/string_number_conversions.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "neva/app_runtime/webview_profile.h"
#include "neva/wam_demo/wam_demo_webview_observer.h"

namespace wam_demo {

bool WamDemoWebView::policy_ = false;
const char kErrorPageFileName[] = "wam_demo_network_error_page.html";

static std::string GetErrorPageBaseUrl() {
  std::string error_page_base_url = "file://";
  base::FilePath path;

  if (!(base::PathService::Get(base::DIR_EXE, &path) &&
        base::PathExists(path))) {
    return {};
  }

  error_page_base_url += path.value() + "/";
  error_page_base_url += kErrorPageFileName;

  return error_page_base_url;
}

WamDemoWebView::WamDemoWebView(std::string appid,
                               WamDemoWebViewObserver* observer)
    : observer_(observer),
      error_page_base_url_(GetErrorPageBaseUrl()),
      appid_(std::move(appid)) {}

WamDemoWebView::WamDemoWebView(std::string appid,
                               int width,
                               int height,
                               WamDemoWebViewObserver* observer,
                               neva_app_runtime::WebViewProfile* profile)
    : WebViewBase(width, height, profile),
      observer_(observer),
      error_page_base_url_(GetErrorPageBaseUrl()),
      appid_(std::move(appid)) {}

WamDemoWebView::~WamDemoWebView() = default;

void WamDemoWebView::OnLoadProgressChanged(double progress) {
  progress_ = base::NumberToString((int)std::floor(progress * 100 + 0.5));

  std::string progress_title = title_;
  if (progress_ != "100")
    progress_title += " " + progress_ + "%";

  if (observer_)
    observer_->OnTitleChanged(this, progress_title);

  LOG(INFO) << __func__ << "(): " << progress_title;
}

void WamDemoWebView::DidFirstFrameFocused() {
  LOG(INFO) << __func__ << "(): Did frame focused is delivered";
}

void WamDemoWebView::DidLoadingEnd(int64_t loading_end_us) {
  LOG(INFO) << __func__ << "(): Loading end is delivered";

  if (observer_)
    observer_->OnDidLoadingEnd(this, loading_end_us);
}

void WamDemoWebView::DidFirstPaint(int64_t first_paint_us) {
  LOG(INFO) << __func__ << "(): First paint is delivered";

  if (observer_)
    observer_->OnDidFirstPaint(this, first_paint_us);
}

void WamDemoWebView::DidFirstContentfulPaint(
    int64_t first_contentful_paint_us) {
  LOG(INFO) << __func__ << "(): First contentful paint is delivered";

  if (observer_)
    observer_->OnDidFirstContentfulPaint(this, first_contentful_paint_us);
}

void WamDemoWebView::DidFirstImagePaint(int64_t first_image_paint_us) {
  LOG(INFO) << __func__ << "(): First image paint is delivered";

  if (observer_)
    observer_->OnDidFirstImagePaint(this, first_image_paint_us);
}

void WamDemoWebView::DidFirstMeaningfulPaint(
    int64_t first_meaningful_paint_us) {
  LOG(INFO) << __func__ << "(): First meaningful paint is delivered";

  if (observer_)
    observer_->OnDidFirstMeaningfulPaint(this, first_meaningful_paint_us);
}

void WamDemoWebView::DidLargestContentfulPaint(
    int64_t largest_contentful_paint_us) {
  LOG(INFO) << __func__ << "(): Largest contentful paint is delivered";

  if (observer_)
    observer_->OnDidLargestContentfulPaint(this, largest_contentful_paint_us);
}

void WamDemoWebView::TitleChanged(const std::string& title) {
  title_ = title;
  std::string progress_title = title;
  if (progress_ != "100")
    progress_title += " " + progress_ + "%";

  if (observer_)
    observer_->OnTitleChanged(this, progress_title);
}

void WamDemoWebView::NavigationHistoryChanged() {
  LOG(INFO) << __func__
            << "(): Navigation history changed notification is delivered";
}

void WamDemoWebView::Close() {
  if (observer_)
    observer_->OnClose(this);
}

bool WamDemoWebView::DecidePolicyForResponse(bool is_main_frame,
                                        int status_code,
                                        const std::string& url,
                                        const std::string& status_text) {
  return policy_;
}

void WamDemoWebView::SetDecidePolicyForResponse() {
  policy_ = true;
}

void WamDemoWebView::SetMediaCapturePermission() {
  LOG(INFO) << __func__ << "(): Set media capture permission is delivered";
  accepts_video_capture_ = accepts_audio_capture_ = true;
}

void WamDemoWebView::ClearMediaCapturePermission() {
  LOG(INFO) << __func__ << "(): Clear media capture permission is delivered";
  accepts_video_capture_ = accepts_audio_capture_ = false;
}

bool WamDemoWebView::AcceptsVideoCapture() {
  LOG(INFO) << __func__ << "(): Accepts video capture is delivered";
  return accepts_video_capture_;
}

bool WamDemoWebView::AcceptsAudioCapture() {
  LOG(INFO) << __func__ << "(): Accepts audio capture is delivered";
  return accepts_audio_capture_;
}

void WamDemoWebView::LoadStarted() {
  LOG(INFO) << __func__ << "(): Load started notification is delivered";
}

void WamDemoWebView::LoadFinished(const std::string& url) {
  LOG(INFO) << __func__
            << "(): Load finished notification is delivered"
            << " for url [" << url << "]";
}

void WamDemoWebView::LoadFailed(const std::string& url,
                           int error_code,
                           const std::string& error_description) {
  LOG(INFO) << __func__
            << "(): Load failed notification is delivered"
            << " for url [" << url << "]";

  // Load error page if url is not error page
  if (url.find(error_page_base_url_) == std::string::npos) {
    load_failed_url_ = url;

    LoadErrorPage(error_code);
  }

  if (observer_)
    observer_->OnLoadFailed(this);
}

void WamDemoWebView::LoadErrorPage(int error_code) {
  std::string err_page_url = error_page_base_url_;
  std::string lang;
  std::string param;

  auto* renderer_prefs(GetWebContents()->GetMutableRendererPrefs());

  if (renderer_prefs) {
    std::vector<std::string> languages =
        base::SplitString(renderer_prefs->accept_languages, ",",
                          base::TRIM_WHITESPACE, base::SPLIT_WANT_NONEMPTY);

    if (languages.size() > 0) {
      lang = languages[0];
    }
  }

  if (!lang.empty()) {
    param += "?language=";
    param += lang;
  }

  if (error_code) {
    param += param.empty() ? "?errcode=" + std::to_string(error_code)
                           : "&errcode=" + std::to_string(error_code);
  }

  err_page_url += param;

  LoadUrl(err_page_url);
}

bool WamDemoWebView::LoadFailedUrlIfNeccessary() {
  if (load_failed_url_.empty()) {
    return false;
  }

  LoadUrl(load_failed_url_);
  return true;
}

void WamDemoWebView::Reload() {
  if (!LoadFailedUrlIfNeccessary()) {
    WebViewBase::Reload();
  }
}

void WamDemoWebView::LoadAborted(const std::string& url) {
  LOG(INFO) << __func__
            << "(): Load aborted notification is delivered"
            << " for url [" << url << "]";
  if (observer_)
    observer_->OnLoadFailed(this);
}

void WamDemoWebView::LoadStopped() {
  LOG(INFO) << __func__ << "(): Load stopped notification is delivered";
}

void WamDemoWebView::RenderProcessCreated(int pid) {
  if (observer_)
    observer_->OnRenderProcessCreated(this);
}

void WamDemoWebView::RenderProcessGone() {
  if (observer_)
    observer_->OnRenderProcessGone(this);
}

void WamDemoWebView::DocumentLoadFinished() {
  LOG(INFO) << __func__
            << "(): Document load finished notification is delivered";
  if (observer_)
    observer_->OnDocumentLoadFinished(this);
}

void WamDemoWebView::DidStartNavigation(const std::string& url, bool is_main_frame) {
  LOG(INFO) << __func__
            << "(): Did start navigation notification is delivered"
            << " for url [" << url << "]";
}

void WamDemoWebView::DidFinishNavigation(const std::string& url, bool is_main_frame) {
  LOG(INFO) << __func__
            << "(): Did finish navigation notification is delivered"
            << " for url [" << url << "]";

  if (url.find(error_page_base_url_) == std::string::npos) {
    load_failed_url_.clear();
  }
}

void WamDemoWebView::DidHistoryBackOnTopPage() {
  LOG(INFO) << __func__
            << "(): Did history back on top page notification is delivered";
}

void WamDemoWebView::DidClearWindowObject() {
  LOG(INFO) << __func__
            << "(): Did clear window object notification is delivered";
}

void WamDemoWebView::DidSwapCompositorFrame() {
  LOG(INFO) << __func__
            << "(): Did swap window frame notification is delivered";
}

void WamDemoWebView::DidResumeDOM() {
  LOG(INFO) << __func__ << "(): Did resume DOM notification is delivered";
}

void WamDemoWebView::DidDropAllPeerConnections(
    neva_app_runtime::DropPeerConnectionReason reason) {}

void WamDemoWebView::RunCommand(const std::string& name,
                           const std::vector<std::string>& arguments) {
  if (observer_)
    observer_->OnBrowserControlCommand(this, name, arguments);
}

std::string WamDemoWebView::RunFunction(const std::string& name,
                            const std::vector<std::string>& arguments) {
  return observer_ ? observer_->OnBrowserControlFunction(this, name, arguments)
                   : std::string();
}

}  // namespace wam_demo
