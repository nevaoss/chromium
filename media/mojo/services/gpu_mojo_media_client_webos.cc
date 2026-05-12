// Copyright 2023 LG Electronics, Inc.
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

#include "media/mojo/services/gpu_mojo_media_client.h"

#include "media/gpu/ipc/service/vda_video_decoder.h"

namespace media {

class GpuMojoMediaClientWebOS final : public GpuMojoMediaClient {
 public:
  GpuMojoMediaClientWebOS(GpuMojoMediaClientTraits& traits)
      : GpuMojoMediaClient(traits) {}
  ~GpuMojoMediaClientWebOS() final = default;

 protected:
  std::unique_ptr<VideoDecoder> CreatePlatformVideoDecoder(
      VideoDecoderTraits& traits) final {
    if (traits.oop_video_decoder) {
      return nullptr;
    }

    VLOG(1) << "Create VdaVideoDecoder for webOS";
    return VdaVideoDecoder::Create(
        traits.task_runner, gpu_task_runner_, traits.media_log->Clone(),
        *traits.target_color_space, gpu_preferences_, gpu_workarounds_,
        traits.get_command_buffer_stub_cb,
        VideoDecodeAccelerator::Config::OutputMode::kAllocate);
  }

  std::optional<SupportedVideoDecoderConfigs>
  GetPlatformSupportedVideoDecoderConfigs(
      GetVdaConfigsCB get_vda_configs) final {
    return std::move(get_vda_configs).Run();
  }

  VideoDecoderType GetPlatformDecoderImplementationType() final {
    return VideoDecoderType::kVda;
  }
};

std::unique_ptr<GpuMojoMediaClient> CreateGpuMediaService(
    GpuMojoMediaClientTraits& traits) {
  return std::make_unique<GpuMojoMediaClientWebOS>(traits);
}

}  // namespace media
