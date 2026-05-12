// Copyright 2017-2020 LG Electronics, Inc.
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

#ifndef NEVA_WAM_DEMO_WAM_DEMO_WEBVIEW_OBSERVER_H_
#define NEVA_WAM_DEMO_WAM_DEMO_WEBVIEW_OBSERVER_H_

#include <string>
#include <vector>

namespace wam_demo {

class WamDemoWebView;

class WamDemoWebViewObserver {
 public:
  virtual void OnDocumentLoadFinished(WamDemoWebView* view) = 0;
  virtual void OnLoadFailed(WamDemoWebView* view) = 0;
  virtual void OnRenderProcessGone(WamDemoWebView* view) = 0;
  virtual void OnRenderProcessCreated(WamDemoWebView* view) = 0;
  virtual void OnDidLoadingEnd(WamDemoWebView* view,
                               int64_t loading_end_us) = 0;
  virtual void OnDidFirstPaint(WamDemoWebView* view,
                               int64_t first_paint_us) = 0;
  virtual void OnDidFirstContentfulPaint(WamDemoWebView* view,
                                         int64_t first_contentful_paint_us) = 0;
  virtual void OnDidFirstImagePaint(WamDemoWebView* view,
                                    int64_t first_image_paint_us) = 0;
  virtual void OnDidFirstMeaningfulPaint(WamDemoWebView* view,
                                         int64_t first_meaningful_paint_us) = 0;
  virtual void OnDidLargestContentfulPaint(
      WamDemoWebView* view,
      int64_t largest_contentful_paint_us) = 0;
  virtual void OnTitleChanged(WamDemoWebView* view,
                              const std::string& title) = 0;
  virtual void OnBrowserControlCommand(
      WamDemoWebView* view,
      const std::string& name,
      const std::vector<std::string>& args) = 0;
  virtual std::string OnBrowserControlFunction(
      WamDemoWebView* view,
      const std::string& name,
      const std::vector<std::string>& args) = 0;
  virtual void OnClose(WamDemoWebView* view) = 0;
};

}  // namespace wam_demo

#endif  // NEVA_WAM_DEMO_WAM_DEMO_WEBVIEW_OBSERVER_H_
