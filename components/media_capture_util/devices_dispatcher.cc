// Copyright 2018 LG Electronics, Inc.
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

#include "components/media_capture_util/devices_dispatcher.h"

#include "base/functional/callback.h"
#include "base/logging.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/media_capture_devices.h"
#include "third_party/blink/public/mojom/mediastream/media_stream.mojom.h"

using content::BrowserThread;

namespace media_capture_util {
namespace {

// Finds a device in |devices| that has |device_id|, or nullptr if not found.
const blink::MediaStreamDevice* FindDeviceWithId(
    const blink::MediaStreamDevices& devices,
    const std::string& device_id) {
  blink::MediaStreamDevices::const_iterator iter = devices.begin();
  for (; iter != devices.end(); ++iter) {
    if (iter->id == device_id) {
      return &(*iter);
    }
  }
  return nullptr;
}

// TODO(neva): Revise solution to support multiple devices in MediaStreamRequest
const blink::MediaStreamDevice* GetAudioDevice(
    const std::vector<std::string>& requested_device_ids) {
  for (const auto& requested_device_id : requested_device_ids) {
    if (requested_device_id.empty()) {
      continue;
    }

    const blink::MediaStreamDevices& audio_devices =
        content::MediaCaptureDevices::GetInstance()->GetAudioCaptureDevices();

    const blink::MediaStreamDevice* audio_device =
        FindDeviceWithId(audio_devices, requested_device_id);
    if (audio_device) {
      return audio_device;
    }
  }
  return nullptr;
}

// TODO(neva): Revise solution to support multiple devices in MediaStreamRequest
const blink::MediaStreamDevice* GetVideoDevice(
    const std::vector<std::string>& requested_video_device_ids) {
  for (const auto& requested_video_device_id : requested_video_device_ids) {
    if (requested_video_device_id.empty()) {
      continue;
    }

    const blink::MediaStreamDevices& video_devices =
        content::MediaCaptureDevices::GetInstance()->GetVideoCaptureDevices();

    const blink::MediaStreamDevice* video_device =
        FindDeviceWithId(video_devices, requested_video_device_id);
    if (video_device) {
      return video_device;
    }
  }
  return nullptr;
}

}  // namespace

DevicesDispatcher* DevicesDispatcher::GetInstance() {
  return base::Singleton<DevicesDispatcher>::get();
}

void DevicesDispatcher::ProcessMediaAccessRequest(
    content::WebContents* web_contents,
    const content::MediaStreamRequest& request,
    bool accepts_video,
    bool accepts_audio,
    content::MediaResponseCallback callback) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  std::unique_ptr<content::MediaStreamUI> ui;

  // TOOD(crbug.com/1300883): Generalize to multiple streams.
  blink::mojom::StreamDevicesSet stream_devices_set;
  stream_devices_set.stream_devices.emplace_back(
      blink::mojom::StreamDevices::New());

  blink::mojom::StreamDevices& stream_devices =
      *stream_devices_set.stream_devices[0];

  if (request.audio_type ==
          blink::mojom::MediaStreamType::DEVICE_AUDIO_CAPTURE &&
      accepts_audio) {
    const blink::MediaStreamDevice* device =
        GetAudioDevice(request.requested_audio_device_ids);
    if (device)
      stream_devices.audio_device = *device;
  }
  if (request.video_type ==
          blink::mojom::MediaStreamType::DEVICE_VIDEO_CAPTURE &&
      accepts_video) {
    const blink::MediaStreamDevice* device =
        GetVideoDevice(request.requested_video_device_ids);
    if (device)
      stream_devices.video_device = *device;
  }

  bool is_devices_empty = !stream_devices.audio_device.has_value() &&
                          !stream_devices.video_device.has_value();

  if (is_devices_empty)
    stream_devices_set.stream_devices.clear();

  std::move(callback).Run(
      stream_devices_set,
      (is_devices_empty ? blink::mojom::MediaStreamRequestResult::NO_HARDWARE
                        : blink::mojom::MediaStreamRequestResult::OK),
      std::move(ui));
}

}  // namespace media_capture_util
