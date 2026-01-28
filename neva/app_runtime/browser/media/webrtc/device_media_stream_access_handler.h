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
// Based on
// chrome/browser/media/webrtc/permission_bubble_media_access_handler.h
//
// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NEVA_APP_RUNTIME_BROWSER_MEDIA_WEBRTC_DEVICE_MEDIA_STREAM_ACCESS_HANDLER_H_
#define NEVA_APP_RUNTIME_BROWSER_MEDIA_WEBRTC_DEVICE_MEDIA_STREAM_ACCESS_HANDLER_H_

#include "base/memory/weak_ptr.h"
#include "components/content_settings/core/common/content_settings.h"
#include "neva/app_runtime/browser/media/media_access_handler.h"
#include "neva/app_runtime/browser/web_contents_collection.h"
#include "third_party/blink/public/mojom/mediastream/media_stream.mojom-shared.h"
#include "third_party/blink/public/mojom/mediastream/media_stream.mojom.h"

namespace user_prefs {
class PrefRegistrySyncable;
}

namespace neva_app_runtime {

// MediaAccessHandler for mic/camera device permission requests.
class DeviceMediaStreamAccessHandler : public MediaAccessHandler,
                                       public WebContentsCollection::Observer {
 public:
  DeviceMediaStreamAccessHandler();
  ~DeviceMediaStreamAccessHandler() override;

  DeviceMediaStreamAccessHandler(const DeviceMediaStreamAccessHandler&) =
      delete;
  DeviceMediaStreamAccessHandler& operator=(
      const DeviceMediaStreamAccessHandler&) = delete;

  // MediaAccessHandler implementation.
  bool SupportsStreamType(content::WebContents* web_contents,
                          const blink::mojom::MediaStreamType type) override;
  bool CheckMediaAccessPermission(content::RenderFrameHost* render_frame_host,
                                  const url::Origin& security_origin,
                                  blink::mojom::MediaStreamType type) override;
  void HandleRequest(content::WebContents* web_contents,
                     const content::MediaStreamRequest& request,
                     content::MediaResponseCallback callback) override;
  void UpdateMediaRequestState(int render_process_id,
                               int render_frame_id,
                               int page_request_id,
                               blink::mojom::MediaStreamType stream_type,
                               content::MediaRequestState state) override;

  // Registers the prefs backing the audio and video policies.
  static void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry);

 private:
  struct PendingAccessRequest;
  using RequestsMap = std::map<int64_t, PendingAccessRequest>;
  using RequestsMaps = std::map<content::WebContents*, RequestsMap>;

  void ProcessQueuedAccessRequest(content::WebContents* web_contents);
  void OnMediaStreamRequestResponse(
      content::WebContents* web_contents,
      int64_t request_id,
      const content::MediaStreamRequest& request,
      const blink::mojom::StreamDevicesSet& devices,
      blink::mojom::MediaStreamRequestResult result,
      bool blocked_by_permissions_policy,
      ContentSetting audio_setting,
      ContentSetting video_setting);
  void OnAccessRequestResponse(
      content::WebContents* web_contents,
      int64_t request_id,
      const blink::mojom::StreamDevicesSet& stream_devices_set,
      blink::mojom::MediaStreamRequestResult result,
      std::unique_ptr<content::MediaStreamUI> ui);

  // WebContentsCollection::Observer:
  void WebContentsDestroyed(content::WebContents* web_contents) override;

  int64_t next_request_id_ = 0;
  RequestsMaps pending_requests_;

  WebContentsCollection web_contents_collection_;

  base::WeakPtrFactory<DeviceMediaStreamAccessHandler> weak_factory_{this};
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_BROWSER_MEDIA_WEBRTC_DEVICE_MEDIA_STREAM_ACCESS_HANDLER_H_
