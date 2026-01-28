// Copyright 2025 LG Electronics, Inc.
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
//
// Based on chrome/browser/media/media_access_handler.h
//
// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NEVA_APP_RUNTIME_BROWSER_MEDIA_MEDIA_ACCESS_HANDLER_H_
#define NEVA_APP_RUNTIME_BROWSER_MEDIA_MEDIA_ACCESS_HANDLER_H_

#include "base/functional/callback.h"
#include "content/public/browser/media_request_state.h"
#include "content/public/browser/media_stream_request.h"

namespace content {
class RenderFrameHost;
class WebContents;
}  // namespace content

namespace neva_app_runtime {

class MediaAccessHandler {
 public:
  MediaAccessHandler() = default;
  virtual ~MediaAccessHandler() = default;

  virtual bool SupportsStreamType(content::WebContents* web_contents,
                                  const blink::mojom::MediaStreamType type) = 0;

  virtual bool CheckMediaAccessPermission(
      content::RenderFrameHost* render_frame_host,
      const url::Origin& security_origin,
      blink::mojom::MediaStreamType type) = 0;

  virtual void HandleRequest(content::WebContents* web_contents,
                             const content::MediaStreamRequest& request,
                             content::MediaResponseCallback callback) = 0;

  virtual void UpdateMediaRequestState(
      int render_process_id,
      int render_frame_id,
      int page_request_id,
      blink::mojom::MediaStreamType stream_type,
      content::MediaRequestState state) {}

  virtual bool IsInsecureCapturingInProgress(int render_process_id,
                                             int render_frame_id);

  virtual void UpdateVideoScreenCaptureStatus(int render_process_id,
                                              int render_frame_id,
                                              int page_request_id,
                                              bool is_secure) {}
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_BROWSER_MEDIA_MEDIA_ACCESS_HANDLER_H_
