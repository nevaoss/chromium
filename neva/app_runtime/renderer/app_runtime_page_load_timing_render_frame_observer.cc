// Copyright 2017-2019 LG Electronics, Inc.
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

#include "neva/app_runtime/renderer/app_runtime_page_load_timing_render_frame_observer.h"

#include <optional>

#include "base/time/time.h"
#include "neva/app_runtime/public/mojom/app_runtime_webview.mojom.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_provider.h"
#include "third_party/blink/public/web/web_local_frame.h"
#include "third_party/blink/public/web/web_performance_metrics_for_reporting.h"

namespace neva_app_runtime {
namespace {

base::TimeDelta ClampDelta(double event, double start) {
  if (event - start < 0)
    event = start;
  return base::Time::FromSecondsSinceUnixEpoch(event) -
         base::Time::FromSecondsSinceUnixEpoch(start);
}

}  // namespace

AppRuntimePageLoadTimingRenderFrameObserver::
    AppRuntimePageLoadTimingRenderFrameObserver(
        content::RenderFrame* render_frame)
    : content::RenderFrameObserver(render_frame) {}

AppRuntimePageLoadTimingRenderFrameObserver::
    ~AppRuntimePageLoadTimingRenderFrameObserver() {}

void AppRuntimePageLoadTimingRenderFrameObserver::DidChangePerformanceTiming() {
  // Check frame exists
  if (HasNoRenderFrame()) {
    return;
  }

  if (PageLoadTimingIsLoadingEnd()) {
    mojo::AssociatedRemote<mojom::AppRuntimeWebViewHost> interface;
    render_frame()->GetRemoteAssociatedInterfaces()->GetInterface(&interface);
    if (interface)
      interface->DidLoadingEnd(*loading_end_);
  }

  if (PageLoadTimingIsFirstPaint()) {
    mojo::AssociatedRemote<mojom::AppRuntimeWebViewHost> interface;
    render_frame()->GetRemoteAssociatedInterfaces()->GetInterface(&interface);
    if (interface)
      interface->DidFirstPaint(*first_paint_);
  }

  if (PageLoadTimingIsFirstContentfulPaint()) {
    mojo::AssociatedRemote<mojom::AppRuntimeWebViewHost> interface;
    render_frame()->GetRemoteAssociatedInterfaces()->GetInterface(&interface);
    if (interface)
      interface->DidFirstContentfulPaint(*first_contentful_paint_);
  }

  if (PageLoadTimingIsFirstImagePaint()) {
    mojo::AssociatedRemote<mojom::AppRuntimeWebViewHost> interface;
    render_frame()->GetRemoteAssociatedInterfaces()->GetInterface(&interface);
    if (interface)
      interface->DidFirstImagePaint(*first_image_paint_);
  }

  if (PageLoadTimingIsFirstMeaningfulPaint()) {
    mojo::AssociatedRemote<mojom::AppRuntimeWebViewHost> interface;
    render_frame()->GetRemoteAssociatedInterfaces()->GetInterface(&interface);
    if (interface)
      interface->DidFirstMeaningfulPaint(*first_meaningful_paint_);
  }

  if (PageLoadTimingIsLargestContentfulPaint()) {
    mojo::AssociatedRemote<mojom::AppRuntimeWebViewHost> interface;
    render_frame()->GetRemoteAssociatedInterfaces()->GetInterface(&interface);
    if (interface)
      interface->DidLargestContentfulPaint(*largest_contentful_paint_);
  }
}

void AppRuntimePageLoadTimingRenderFrameObserver::OnDestruct() {
  delete this;
}

bool AppRuntimePageLoadTimingRenderFrameObserver::HasNoRenderFrame() const {
  bool no_frame = !render_frame() || !render_frame()->GetWebFrame();
  DCHECK(!no_frame);
  return no_frame;
}

bool AppRuntimePageLoadTimingRenderFrameObserver::
    PageLoadTimingIsLoadingEnd() {
  if (loading_end_)
    return false;

  if (HasNoRenderFrame()) {
    return false;
  }

  const blink::WebPerformanceMetricsForReporting& perf =
      render_frame()->GetWebFrame()->PerformanceMetricsForReporting();

  if (perf.LoadEventEnd() > 0.0) {
    loading_end_ =
        ClampDelta(perf.LoadEventEnd(), perf.NavigationStart());
    return true;
  }
  return false;
}

bool AppRuntimePageLoadTimingRenderFrameObserver::
    PageLoadTimingIsFirstPaint() {
  if (first_paint_)
    return false;

  if (HasNoRenderFrame()) {
    return false;
  }

  const blink::WebPerformanceMetricsForReporting& perf =
      render_frame()->GetWebFrame()->PerformanceMetricsForReporting();

  if (perf.FirstPaint() > 0.0) {
    first_paint_ =
        ClampDelta(perf.FirstPaint(), perf.NavigationStart());
    return true;
  }
  return false;
}

bool AppRuntimePageLoadTimingRenderFrameObserver::
    PageLoadTimingIsFirstContentfulPaint() {
  if (first_contentful_paint_)
    return false;

  if (HasNoRenderFrame()) {
    return false;
  }

  const blink::WebPerformanceMetricsForReporting& perf =
      render_frame()->GetWebFrame()->PerformanceMetricsForReporting();

  if (perf.FirstContentfulPaint() > 0.0) {
    first_contentful_paint_ =
        ClampDelta(perf.FirstContentfulPaint(), perf.NavigationStart());
    return true;
  }
  return false;
}

bool AppRuntimePageLoadTimingRenderFrameObserver::
    PageLoadTimingIsFirstImagePaint() {
  if (first_image_paint_)
    return false;

  if (HasNoRenderFrame()) {
    return false;
  }

  const blink::WebPerformanceMetricsForReporting& perf =
      render_frame()->GetWebFrame()->PerformanceMetricsForReporting();

  if (perf.FirstImagePaint() > 0.0) {
    first_image_paint_ =
        ClampDelta(perf.FirstImagePaint(), perf.NavigationStart());
    return true;
  }
  return false;
}

bool AppRuntimePageLoadTimingRenderFrameObserver::
    PageLoadTimingIsFirstMeaningfulPaint() {
  if (first_meaningful_paint_)
    return false;

  if (HasNoRenderFrame()) {
    return false;
  }

  const blink::WebPerformanceMetricsForReporting& perf =
      render_frame()->GetWebFrame()->PerformanceMetricsForReporting();

  if (perf.FirstMeaningfulPaint() > 0.0) {
    first_meaningful_paint_ =
        ClampDelta(perf.FirstMeaningfulPaint(), perf.NavigationStart());
    return true;
  }
  return false;
}

bool AppRuntimePageLoadTimingRenderFrameObserver::
    PageLoadTimingIsLargestContentfulPaint() {
  if (HasNoRenderFrame()) {
    return false;
  }

  const blink::WebPerformanceMetricsForReporting& perf =
      render_frame()->GetWebFrame()->PerformanceMetricsForReporting();
  auto largest_contentful_paint_details =
      perf.LargestContentfulDetailsForMetrics();

  if ((largest_contentful_paint_details.text_paint_size == 0) &&
      (largest_contentful_paint_details.image_paint_size == 0)) {
    return false;
  }

  if ((largest_contentful_paint_details.text_paint_size <=
       largest_contentful_paint_size_) &&
      (largest_contentful_paint_details.image_paint_size <=
       largest_contentful_paint_size_)) {
    return false;
  }

  if (largest_contentful_paint_details.text_paint_size >
          largest_contentful_paint_details.image_paint_size &&
      largest_contentful_paint_details.text_paint_time > 0.0) {
    largest_contentful_paint_ =
        ClampDelta(largest_contentful_paint_details.text_paint_time,
                   perf.NavigationStart());
    largest_contentful_paint_size_ =
        largest_contentful_paint_details.text_paint_size;
  } else if (largest_contentful_paint_details.image_paint_time > 0.0) {
    largest_contentful_paint_ =
        ClampDelta(largest_contentful_paint_details.image_paint_time,
                   perf.NavigationStart());
    largest_contentful_paint_size_ =
        largest_contentful_paint_details.image_paint_size;
  } else {
    return false;
  }

  return true;
}

}  // namespace neva_app_runtime
