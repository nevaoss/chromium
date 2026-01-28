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
// chrome/browser/media/webrtc/media_capture_devices_dispatcher.h
//
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NEVA_APP_RUNTIME_BROWSER_MEDIA_WEBRTC_MEDIA_CAPTURE_DEVICES_DISPATCHER_H_
#define NEVA_APP_RUNTIME_BROWSER_MEDIA_WEBRTC_MEDIA_CAPTURE_DEVICES_DISPATCHER_H_

#include "base/memory/singleton.h"
#include "components/webrtc/media_stream_device_enumerator_impl.h"
#include "content/public/browser/media_observer.h"
#include "content/public/browser/media_stream_request.h"
#include "neva/app_runtime/browser/media/media_access_handler.h"

namespace content {
class BrowserContext;
class RenderFrameHost;
class WebContents;
}  // namespace content

namespace user_prefs {
class PrefRegistrySyncable;
}

namespace neva_app_runtime {

class MediaAccessHandler;
class MediaStreamCaptureIndicator;

class MediaCaptureDevicesDispatcher
    : public content::MediaObserver,
      public webrtc::MediaStreamDeviceEnumeratorImpl {
 public:
  static MediaCaptureDevicesDispatcher* GetInstance();

  static void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry);

  void ProcessMediaAccessRequest(content::WebContents* web_contents,
                                 const content::MediaStreamRequest& request,
                                 content::MediaResponseCallback callback);
  bool CheckMediaAccessPermission(content::RenderFrameHost* render_frame_host,
                                  const url::Origin& security_origin,
                                  blink::mojom::MediaStreamType type);
  std::string GetDefaultDeviceIDForProfile(content::BrowserContext* context,
                                           blink::mojom::MediaStreamType type);

  // webrtc::MediaStreamDeviceEnumeratorImpl:
  const blink::MediaStreamDevices& GetAudioCaptureDevices() const override;
  const blink::MediaStreamDevices& GetVideoCaptureDevices() const override;

  const std::optional<blink::MediaStreamDevice>
  GetPreferredAudioDeviceForBrowserContext(
      content::BrowserContext* context,
      const std::vector<std::string>& eligible_device_ids) const override;

  const std::optional<blink::MediaStreamDevice>
  GetPreferredVideoDeviceForBrowserContext(
      content::BrowserContext* context,
      const std::vector<std::string>& eligible_device_ids) const override;

  // content::MediaObserver:
  void OnMediaRequestStateChanged(int render_process_id,
                                  int render_frame_id,
                                  int page_request_id,
                                  const GURL& security_origin,
                                  blink::mojom::MediaStreamType stream_type,
                                  content::MediaRequestState state) override;
  void OnSetCapturingLinkSecured(int render_process_id,
                                 int render_frame_id,
                                 int page_request_id,
                                 blink::mojom::MediaStreamType stream_type,
                                 bool is_secure) override;
  void OnCreatingAudioStream(int render_process_id,
                             int render_frame_id) override {}
  void OnAudioCaptureDevicesChanged() override {}
  void OnVideoCaptureDevicesChanged() override {}

  scoped_refptr<MediaStreamCaptureIndicator> GetMediaStreamCaptureIndicator();

  bool IsInsecureCapturingInProgress(int render_process_id,
                                     int render_frame_id);

 private:
  friend class MediaCaptureDevicesDispatcherTest;

  friend struct base::DefaultSingletonTraits<MediaCaptureDevicesDispatcher>;

  MediaCaptureDevicesDispatcher();
  ~MediaCaptureDevicesDispatcher() override;

  MediaCaptureDevicesDispatcher(const MediaCaptureDevicesDispatcher&) = delete;
  MediaCaptureDevicesDispatcher& operator=(
      const MediaCaptureDevicesDispatcher&) = delete;

  void UpdateMediaRequestStateOnUIThread(
      int render_process_id,
      int render_frame_id,
      int page_request_id,
      const GURL& security_origin,
      blink::mojom::MediaStreamType stream_type,
      content::MediaRequestState state);
  void UpdateVideoScreenCaptureStatus(int render_process_id,
                                      int render_frame_id,
                                      int page_request_id,
                                      blink::mojom::MediaStreamType stream_type,
                                      bool is_secure);

  scoped_refptr<MediaStreamCaptureIndicator> media_stream_capture_indicator_;

  // Handlers for processing media access requests.
  std::vector<std::unique_ptr<MediaAccessHandler>> media_access_handlers_;
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_BROWSER_MEDIA_WEBRTC_MEDIA_CAPTURE_DEVICES_DISPATCHER_H_
