// Copyright 2022 LG Electronics, Inc.
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

#ifndef MEDIA_GPU_WEBOS_WEBOS_VIDEO_ENCODE_ACCELERATOR_H_
#define MEDIA_GPU_WEBOS_WEBOS_VIDEO_ENCODE_ACCELERATOR_H_

#include <stddef.h>
#include <stdint.h>

#include <mcil/video_encoder_client.h>

#include <memory>
#include <numeric>
#include <vector>

#include "base/containers/circular_deque.h"
#include "base/containers/queue.h"
#include "base/files/scoped_file.h"
#include "base/memory/scoped_refptr.h"
#include "base/memory/weak_ptr.h"
#include "base/sequence_checker.h"
#include "base/threading/thread.h"
#include "base/time/time.h"
#include "media/gpu/chromeos/image_processor.h"
#include "media/gpu/media_gpu_export.h"
#include "media/video/video_encode_accelerator.h"
#include "third_party/abseil-cpp/absl/types/optional.h"
#include "ui/gfx/geometry/size.h"

namespace base {
class SequencedTaskRunner;
}  // namespace base

namespace mcil {
class VideoEncoderAPI;
}  // namespace mcil

namespace media {

class H264Parser;

class MEDIA_GPU_EXPORT WebOSVideoEncodeAccelerator
    : public VideoEncodeAccelerator,
      public mcil::VideoEncoderClient {
 public:
  explicit WebOSVideoEncodeAccelerator();
  ~WebOSVideoEncodeAccelerator() override;

  // VideoEncodeAccelerator implementation.
  VideoEncodeAccelerator::SupportedProfiles GetSupportedProfiles() override;
  bool Initialize(const Config& config,
                  Client* client,
                  std::unique_ptr<MediaLog> media_log) override;
  void Encode(scoped_refptr<VideoFrame> frame, bool force_keyframe) override;
  void UseOutputBitstreamBuffer(BitstreamBuffer buffer) override;
  void RequestEncodingParametersChange(
      const Bitrate& bitrate,
      uint32_t framerate,
      const absl::optional<gfx::Size>& size) override;
  void Destroy() override;
  void Flush(FlushCallback flush_callback) override;
  bool IsFlushSupported() override;

  // mcil::VideoEncoderClient implementation.
  void CreateInputBuffers(size_t count) override;
  void DestroyInputBuffers() override;
  void EnqueueInputBuffer(size_t buffer_index) override;
  void DequeueInputBuffer(size_t buffer_index) override;
  void BitstreamBufferReady(mcil::ReadableBufferRef output_buffer) override;
  void BitstreamBufferReady(const uint8_t* bitstream_data,
                            size_t bitstream_size,
                            uint64_t time_stamp,
                            bool is_key_frame) override;
  void PumpBitstreamBuffers() override;
  uint8_t GetH264LevelLimit(const mcil::EncoderConfig* config) override;
  void StopDevicePoll() override;
  void NotifyFlushIfNeeded(bool flush) override;
  void NotifyEncodeBufferTask() override;
  void NotifyEncoderError(mcil::EncoderError error) override;
  void NotifyEncoderState(mcil::CodecState state) override;

 private:
  WebOSVideoEncodeAccelerator(const WebOSVideoEncodeAccelerator&) = delete;
  void operator=(WebOSVideoEncodeAccelerator const&) = delete;

  // Auto-destroy reference for BitstreamBuffer
  struct BitstreamBufferRef;
  // Record for codec input buffers.
  struct InputRecord;
  // Store all the information of input frame passed to Encode().
  struct InputFrameInfo;

  enum {
    kInputBufferCount = 2,
    kImageProcBufferCount = 2,
  };

  void InitializeTask(const Config& config,
                      bool* result,
                      base::WaitableEvent* done);
  void EncodeTask(scoped_refptr<VideoFrame> frame, bool force_keyframe);
  void UseOutputBitstreamBufferTask(BitstreamBuffer buffer);
  void RequestEncodingParametersChangeTask(
      const Bitrate& bitrate,
      uint32_t framerate,
      const absl::optional<gfx::Size>& size);
  void DestroyTask();
  void FlushTask(FlushCallback flush_callback);

  void NotifyError(EncoderStatus status);
  void SetErrorState(EncoderStatus status);

  void SetEncoderState(mcil::CodecState state);

  void Enqueue();
  void RunEncodeBufferTask();

  size_t CopyIntoOutputBuffer(const uint8_t* bitstream_data,
                              size_t bitstream_size,
                              std::unique_ptr<BitstreamBufferRef> buffer_ref);
  void FeedBufferOnEncoderThread();

  bool CreateImageProcessor(const VideoFrameLayout& input_layout,
                            const VideoPixelFormat output_format,
                            const gfx::Size& output_size,
                            const gfx::Rect& input_visible_rect,
                            const gfx::Rect& output_visible_rect);
  bool AllocateImageProcessorOutputBuffers(
      absl::optional<VideoFrameLayout> output_layout,
      size_t count);
  void InputImageProcessorTask();
  void MaybeFlushImageProcessor();
  void ReuseImageProcessorOutputBuffer(size_t output_buffer_index);
  void FrameProcessed(bool force_keyframe,
                      base::TimeDelta timestamp,
                      size_t output_buffer_index,
                      scoped_refptr<VideoFrame> video_frame);
  void ImageProcessorError();
  mcil::scoped_refptr<mcil::VideoFrame> ToMcilFrame(
      scoped_refptr<VideoFrame> video_frame);

  const scoped_refptr<base::SequencedTaskRunner> child_task_runner_;
  SEQUENCE_CHECKER(child_sequence_checker_);

  gfx::Size input_frame_size_;

  gfx::Rect encoder_input_visible_rect_;

  size_t output_buffer_byte_size_ = 0;
  uint32_t output_format_fourcc_ = 0;

  size_t current_bitrate_ = 0;
  size_t current_framerate_ = 0;

  mcil::CodecState encoder_state_ = mcil::kUninitialized;

  base::queue<InputFrameInfo> encoder_input_queue_;
  std::vector<std::unique_ptr<BitstreamBufferRef>> bitstream_buffer_pool_;

  base::circular_deque<mcil::ReadableBufferRef> output_buffer_queue_;

  std::vector<uint8_t> cached_sps_;
  std::vector<uint8_t> cached_pps_;
  size_t cached_h264_header_size_ = 0;
  bool inject_sps_and_pps_ = false;

  bool should_control_buffer_feed_ = false;

  FlushCallback flush_callback_;

  std::vector<InputRecord> input_buffer_map_;

  std::unique_ptr<ImageProcessor> image_processor_;
  std::vector<scoped_refptr<VideoFrame>> image_processor_output_buffers_;
  base::queue<InputFrameInfo> image_processor_input_queue_;
  size_t num_frames_in_image_processor_ = 0;
  std::vector<size_t> free_image_processor_output_buffer_indices_;

  absl::optional<VideoFrameLayout> device_input_layout_;

  // The underlying perform encoding on.
  std::unique_ptr<mcil::VideoEncoderAPI> video_encoder_api_;

  const scoped_refptr<base::SequencedTaskRunner> encoder_task_runner_;
  SEQUENCE_CHECKER(encoder_sequence_checker_);

  base::WeakPtr<Client> client_;
  std::unique_ptr<base::WeakPtrFactory<Client>> client_ptr_factory_;

  base::TimeTicks old_time_ = base::TimeTicks::Now();
  int32_t frames_per_sec_ = 0;

  base::WeakPtr<WebOSVideoEncodeAccelerator> weak_this_;
  base::WeakPtrFactory<WebOSVideoEncodeAccelerator> weak_this_factory_{this};
};

}  // namespace media

#endif  // MEDIA_GPU_WEBOS_WEBOS_VIDEO_ENCODE_ACCELERATOR_H_
