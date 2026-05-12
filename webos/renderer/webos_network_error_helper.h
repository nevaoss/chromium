// Copyright 2019 LG Electronics, Inc.
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

#ifndef WEBOS_RENDERER_WEBOS_NETWORK_ERROR_HELPER_H_
#define WEBOS_RENDERER_WEBOS_NETWORK_ERROR_HELPER_H_

#include "base/memory/weak_ptr.h"
#include "content/public/renderer/render_frame.h"
#include "content/public/renderer/render_frame_observer.h"
#include "content/public/renderer/render_frame_observer_tracker.h"

namespace webos {

class WebOSNetworkErrorHelper
    : public content::RenderFrameObserver,
      public content::RenderFrameObserverTracker<WebOSNetworkErrorHelper> {
 public:
  explicit WebOSNetworkErrorHelper(content::RenderFrame* render_frame);
  WebOSNetworkErrorHelper(const WebOSNetworkErrorHelper&) = delete;
  WebOSNetworkErrorHelper& operator=(const WebOSNetworkErrorHelper&) = delete;

  // RenderFrameObserver implementation
  void DidCommitProvisionalLoad(ui::PageTransition transition) override;
  void DidFinishLoad() override;
  void OnDestruct() override;
};

}  // namespace webos

#endif  // WEBOS_RENDERER_WEBOS_NETWORK_ERROR_HELPER_H_
