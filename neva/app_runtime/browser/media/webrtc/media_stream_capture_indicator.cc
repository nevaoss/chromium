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
// chrome/browser/media/webrtc/media_stream_capture_indicator.cc
//
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "neva/app_runtime/browser/media/webrtc/media_stream_capture_indicator.h"

#include "components/url_formatter/elide_url.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_observer.h"

namespace neva_app_runtime {

namespace {

bool IsDeviceCapturingDisplay(const blink::MediaStreamDevice& device) {
  return device.display_media_info &&
         device.display_media_info->display_surface ==
             media::mojom::DisplayCaptureSurfaceType::MONITOR;
}

typedef void (MediaStreamCaptureIndicator::Observer::*ObserverMethod)(
    content::WebContents* web_contents,
    bool value);

ObserverMethod GetObserverMethodToCall(const blink::MediaStreamDevice& device) {
  switch (device.type) {
    case blink::mojom::MediaStreamType::DEVICE_AUDIO_CAPTURE:
      return &MediaStreamCaptureIndicator::Observer::OnIsCapturingAudioChanged;

    case blink::mojom::MediaStreamType::DEVICE_VIDEO_CAPTURE:
      return &MediaStreamCaptureIndicator::Observer::OnIsCapturingVideoChanged;

    case blink::mojom::MediaStreamType::GUM_TAB_AUDIO_CAPTURE:
    case blink::mojom::MediaStreamType::GUM_TAB_VIDEO_CAPTURE:
      return &MediaStreamCaptureIndicator::Observer::OnIsBeingMirroredChanged;

    case blink::mojom::MediaStreamType::GUM_DESKTOP_VIDEO_CAPTURE:
    case blink::mojom::MediaStreamType::GUM_DESKTOP_AUDIO_CAPTURE:
    case blink::mojom::MediaStreamType::DISPLAY_VIDEO_CAPTURE:
    case blink::mojom::MediaStreamType::DISPLAY_AUDIO_CAPTURE:
    case blink::mojom::MediaStreamType::DISPLAY_VIDEO_CAPTURE_THIS_TAB:
    case blink::mojom::MediaStreamType::DISPLAY_VIDEO_CAPTURE_SET:
      return IsDeviceCapturingDisplay(device)
                 ? &MediaStreamCaptureIndicator::Observer::
                       OnIsCapturingDisplayChanged
                 : &MediaStreamCaptureIndicator::Observer::
                       OnIsCapturingWindowChanged;

    case blink::mojom::MediaStreamType::NO_SERVICE:
    case blink::mojom::MediaStreamType::NUM_MEDIA_TYPES:
      NOTREACHED();
  }
}

}  // namespace

class MediaStreamCaptureIndicator::WebContentsDeviceUsage
    : public content::WebContentsObserver {
 public:
  WebContentsDeviceUsage(scoped_refptr<MediaStreamCaptureIndicator> indicator,
                         content::WebContents* web_contents)
      : WebContentsObserver(web_contents), indicator_(std::move(indicator)) {}

  WebContentsDeviceUsage(const WebContentsDeviceUsage&) = delete;
  WebContentsDeviceUsage& operator=(const WebContentsDeviceUsage&) = delete;

  bool IsCapturingAudio() const { return audio_stream_count_ > 0; }
  bool IsCapturingVideo() const { return video_stream_count_ > 0; }
  bool IsMirroring() const { return mirroring_stream_count_ > 0; }
  bool IsCapturingWindow() const { return window_stream_count_ > 0; }
  bool IsCapturingDisplay() const { return display_stream_count_ > 0; }

  std::unique_ptr<content::MediaStreamUI> RegisterMediaStream(
      const blink::mojom::StreamDevices& devices,
      std::unique_ptr<MediaStreamUI> ui,
      const std::u16string application_title);

  void AddDevices(const blink::mojom::StreamDevices& devices,
                  base::OnceClosure stop_callback);
  void RemoveDevices(const blink::mojom::StreamDevices& devices);

  void NotifyStopped();

 private:
  int& GetStreamCount(const blink::MediaStreamDevice& device);

  void AddDevice(const blink::MediaStreamDevice& device);

  void RemoveDevice(const blink::MediaStreamDevice& device);

  // content::WebContentsObserver overrides.
  void WebContentsDestroyed() override {
    if (web_contents()) {
      indicator_->UnregisterWebContents(web_contents());
    }
  }

  scoped_refptr<MediaStreamCaptureIndicator> indicator_;
  int audio_stream_count_ = 0;
  int video_stream_count_ = 0;
  int mirroring_stream_count_ = 0;
  int window_stream_count_ = 0;
  int display_stream_count_ = 0;

  base::OnceClosure stop_callback_;
  base::WeakPtrFactory<WebContentsDeviceUsage> weak_factory_{this};
};

class MediaStreamCaptureIndicator::UIDelegate : public content::MediaStreamUI {
 public:
  UIDelegate(base::WeakPtr<WebContentsDeviceUsage> device_usage,
             const blink::mojom::StreamDevices& devices,
             std::unique_ptr<::MediaStreamUI> ui,
             const std::u16string application_title)
      : device_usage_(std::move(device_usage)),
        devices_(devices),
        ui_(std::move(ui)),
        application_title_(std::move(application_title)) {
    DCHECK(devices_.audio_device.has_value() ||
           devices_.video_device.has_value());
  }

  UIDelegate(const UIDelegate&) = delete;
  UIDelegate& operator=(const UIDelegate&) = delete;

  ~UIDelegate() override {
    if (started_ && device_usage_.get()) {
      device_usage_->RemoveDevices(devices_);
    }
  }

 private:
  // content::MediaStreamUI interface.
  gfx::NativeViewId OnStarted(
      base::RepeatingClosure stop_callback,
      content::MediaStreamUI::SourceCallback source_callback,
      const std::string& label,
      std::vector<content::DesktopMediaID> screen_capture_ids,
      StateChangeCallback state_change_callback) override {
    if (started_) {
      return 0;
    }

    started_ = true;
    if (device_usage_.get()) {
      device_usage_->AddDevices(devices_,
                                ui_ ? base::OnceClosure() : stop_callback);
    }

    if (ui_) {
      return ui_->OnStarted(std::move(stop_callback),
                            std::move(source_callback));
    }

    return 0;
  }

  void OnDeviceStoppedForSourceChange(
      const std::string& label,
      const content::DesktopMediaID& old_media_id,
      const content::DesktopMediaID& new_media_id,
      bool captured_surface_control_active) override {}

  void OnDeviceStopped(const std::string& label,
                       const content::DesktopMediaID& media_id) override {}

  base::WeakPtr<WebContentsDeviceUsage> device_usage_;
  const blink::mojom::StreamDevices devices_;
  const std::unique_ptr<::MediaStreamUI> ui_;
  const std::u16string application_title_;
  bool started_ = false;
};

std::unique_ptr<content::MediaStreamUI>
MediaStreamCaptureIndicator::WebContentsDeviceUsage::RegisterMediaStream(
    const blink::mojom::StreamDevices& devices,
    std::unique_ptr<MediaStreamUI> ui,
    const std::u16string application_title) {
  return std::make_unique<UIDelegate>(weak_factory_.GetWeakPtr(), devices,
                                      std::move(ui),
                                      std::move(application_title));
}

void MediaStreamCaptureIndicator::WebContentsDeviceUsage::AddDevices(
    const blink::mojom::StreamDevices& devices,
    base::OnceClosure stop_callback) {
  if (devices.audio_device.has_value()) {
    AddDevice(devices.audio_device.value());
  }

  if (devices.video_device.has_value()) {
    AddDevice(devices.video_device.value());
  }

  if (web_contents()) {
    stop_callback_ = std::move(stop_callback);
  }
}

void MediaStreamCaptureIndicator::WebContentsDeviceUsage::RemoveDevices(
    const blink::mojom::StreamDevices& devices) {
  if (devices.audio_device.has_value()) {
    RemoveDevice(devices.audio_device.value());
  }

  if (devices.video_device.has_value()) {
    RemoveDevice(devices.video_device.value());
  }

  if (web_contents() && !web_contents()->IsBeingDestroyed()) {
    web_contents()->NotifyNavigationStateChanged(content::INVALIDATE_TYPE_TAB);
  }
}

void MediaStreamCaptureIndicator::WebContentsDeviceUsage::NotifyStopped() {
  if (stop_callback_) {
    std::move(stop_callback_).Run();
  }
}

int& MediaStreamCaptureIndicator::WebContentsDeviceUsage::GetStreamCount(
    const blink::MediaStreamDevice& device) {
  switch (device.type) {
    case blink::mojom::MediaStreamType::DEVICE_AUDIO_CAPTURE:
      return audio_stream_count_;

    case blink::mojom::MediaStreamType::DEVICE_VIDEO_CAPTURE:
      return video_stream_count_;

    case blink::mojom::MediaStreamType::GUM_TAB_AUDIO_CAPTURE:
    case blink::mojom::MediaStreamType::GUM_TAB_VIDEO_CAPTURE:
      return mirroring_stream_count_;

    case blink::mojom::MediaStreamType::GUM_DESKTOP_VIDEO_CAPTURE:
    case blink::mojom::MediaStreamType::GUM_DESKTOP_AUDIO_CAPTURE:
    case blink::mojom::MediaStreamType::DISPLAY_VIDEO_CAPTURE:
    case blink::mojom::MediaStreamType::DISPLAY_AUDIO_CAPTURE:
    case blink::mojom::MediaStreamType::DISPLAY_VIDEO_CAPTURE_THIS_TAB:
    case blink::mojom::MediaStreamType::DISPLAY_VIDEO_CAPTURE_SET:
      return IsDeviceCapturingDisplay(device) ? display_stream_count_
                                              : window_stream_count_;

    case blink::mojom::MediaStreamType::NO_SERVICE:
    case blink::mojom::MediaStreamType::NUM_MEDIA_TYPES:
      NOTREACHED();
  }
}

void MediaStreamCaptureIndicator::WebContentsDeviceUsage::AddDevice(
    const blink::MediaStreamDevice& device) {
  int& stream_count = GetStreamCount(device);
  ++stream_count;

  if (web_contents() && stream_count == 1) {
    ObserverMethod obs_func = GetObserverMethodToCall(device);
    DCHECK(obs_func);
    for (Observer& obs : indicator_->observers_) {
      (obs.*obs_func)(web_contents(), true);
    }
  }
}

void MediaStreamCaptureIndicator::WebContentsDeviceUsage::RemoveDevice(
    const blink::MediaStreamDevice& device) {
  int& stream_count = GetStreamCount(device);
  --stream_count;
  DCHECK_GE(stream_count, 0);

  if (web_contents() && stream_count == 0) {
    ObserverMethod obs_func = GetObserverMethodToCall(device);
    DCHECK(obs_func);
    for (Observer& obs : indicator_->observers_) {
      (obs.*obs_func)(web_contents(), false);
    }
  }
}

MediaStreamCaptureIndicator::Observer::~Observer() {
  DCHECK(!IsInObserverList());
}

MediaStreamCaptureIndicator::MediaStreamCaptureIndicator() {}

MediaStreamCaptureIndicator::~MediaStreamCaptureIndicator() {
  DCHECK(usage_map_.empty() || !content::BrowserThread::IsThreadInitialized(
                                   content::BrowserThread::UI));
}

std::unique_ptr<content::MediaStreamUI>
MediaStreamCaptureIndicator::RegisterMediaStream(
    content::WebContents* web_contents,
    const blink::mojom::StreamDevices& devices,
    std::unique_ptr<MediaStreamUI> ui,
    const std::u16string application_title) {
  DCHECK(web_contents);
  auto& usage = usage_map_[web_contents];
  if (!usage) {
    usage = std::make_unique<WebContentsDeviceUsage>(this, web_contents);
  }

  return usage->RegisterMediaStream(devices, std::move(ui),
                                    std::move(application_title));
}

bool MediaStreamCaptureIndicator::CheckUsage(
    content::WebContents* web_contents,
    const WebContentsDeviceUsagePredicate& pred) const {
  auto it = usage_map_.find(web_contents);
  if (it != usage_map_.end() && pred.Run(it->second.get())) {
    return true;
  }

  for (auto* inner_contents : web_contents->GetInnerWebContents()) {
    if (CheckUsage(inner_contents, pred)) {
      return true;
    }
  }

  return false;
}

bool MediaStreamCaptureIndicator::IsCapturingUserMedia(
    content::WebContents* web_contents) const {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  return CheckUsage(
      web_contents,
      base::BindRepeating([](const WebContentsDeviceUsage* usage) {
        return usage->IsCapturingAudio() || usage->IsCapturingVideo();
      }));
}

bool MediaStreamCaptureIndicator::IsCapturingVideo(
    content::WebContents* web_contents) const {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  return CheckUsage(
      web_contents,
      base::BindRepeating(&WebContentsDeviceUsage::IsCapturingVideo));
}

bool MediaStreamCaptureIndicator::IsCapturingAudio(
    content::WebContents* web_contents) const {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  return CheckUsage(
      web_contents,
      base::BindRepeating(&WebContentsDeviceUsage::IsCapturingAudio));
}

bool MediaStreamCaptureIndicator::IsBeingMirrored(
    content::WebContents* web_contents) const {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  return CheckUsage(web_contents,
                    base::BindRepeating(&WebContentsDeviceUsage::IsMirroring));
}

bool MediaStreamCaptureIndicator::IsCapturingWindow(
    content::WebContents* web_contents) const {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  return CheckUsage(
      web_contents,
      base::BindRepeating(&WebContentsDeviceUsage::IsCapturingWindow));
}

bool MediaStreamCaptureIndicator::IsCapturingDisplay(
    content::WebContents* web_contents) const {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  return CheckUsage(
      web_contents,
      base::BindRepeating(&WebContentsDeviceUsage::IsCapturingDisplay));
}

void MediaStreamCaptureIndicator::NotifyStopped(
    content::WebContents* web_contents) const {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  auto it = usage_map_.find(web_contents);
  CHECK(it != usage_map_.end());
  it->second->NotifyStopped();

  for (auto* inner_contents : web_contents->GetInnerWebContents()) {
    NotifyStopped(inner_contents);
  }
}

void MediaStreamCaptureIndicator::UnregisterWebContents(
    content::WebContents* web_contents) {
  if (IsCapturingVideo(web_contents)) {
    for (Observer& observer : observers_) {
      observer.OnIsCapturingVideoChanged(web_contents, false);
    }
  }

  if (IsCapturingAudio(web_contents)) {
    for (Observer& observer : observers_) {
      observer.OnIsCapturingAudioChanged(web_contents, false);
    }
  }

  if (IsBeingMirrored(web_contents)) {
    for (Observer& observer : observers_) {
      observer.OnIsBeingMirroredChanged(web_contents, false);
    }
  }

  if (IsCapturingWindow(web_contents)) {
    for (Observer& observer : observers_) {
      observer.OnIsCapturingWindowChanged(web_contents, false);
    }
  }

  if (IsCapturingDisplay(web_contents)) {
    for (Observer& observer : observers_) {
      observer.OnIsCapturingDisplayChanged(web_contents, false);
    }
  }

  usage_map_.erase(web_contents);
}

}  // namespace neva_app_runtime
