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

#include "media/gpu/webos/webos_video_encode_accelerator.h"

#include <mcil/video_encoder_api.h>

#include <optional>

#include "base/bits.h"
#include "base/check_op.h"
#include "base/location.h"
#include "base/memory/shared_memory_mapping.h"
#include "base/task/sequenced_task_runner.h"
#include "base/task/single_thread_task_runner.h"
#include "base/task/task_traits.h"
#include "base/task/thread_pool.h"
#include "build/build_config.h"
#include "media/base/bitstream_buffer.h"
#include "media/gpu/chromeos/image_processor_factory.h"
#include "media/gpu/chromeos/platform_video_frame_utils.h"
#include "media/gpu/gpu_video_encode_accelerator_helpers.h"
#include "media/gpu/webos/webos_codec_utils.h"
#include "media/parsers/h264_level_limits.h"
#include "media/parsers/h264_parser.h"
#include "ui/gl/gl_context.h"
#include "ui/gl/scoped_binders.h"

#define NOTIFY_ERROR(x) \
  do {                  \
    SetErrorState(x);   \
  } while (0)

namespace media {

namespace {

const uint8_t kH264StartCode[] = {0, 0, 0, 1};
const size_t kH264StartCodeSize = sizeof(kH264StartCode);

const int32_t kMaxEncoderFramerate = 30;

static void CopyNALUPrependingStartCode(const uint8_t* src,
                                        size_t src_size,
                                        uint8_t** dst,
                                        size_t* dst_size) {
  size_t size_to_copy = kH264StartCodeSize + src_size;
  if (size_to_copy > *dst_size) {
    LOG(WARNING)
        << __func__
        << " Could not copy a NALU, not enough space in destination buffer";
    return;
  }

  memcpy(*dst, kH264StartCode, kH264StartCodeSize);
  memcpy(*dst + kH264StartCodeSize, src, src_size);

  *dst += size_to_copy;
  *dst_size -= size_to_copy;
}

absl::optional<VideoFrameLayout> AsMultiPlanarLayout(
    const VideoFrameLayout& layout) {
  if (layout.is_multi_planar()) {
    return absl::make_optional<VideoFrameLayout>(layout);
  }
  return VideoFrameLayout::CreateMultiPlanar(
      layout.format(), layout.coded_size(), layout.planes());
}

std::optional<ImageProcessor::PortConfig> VideoFrameLayoutToPortConfig(
    const VideoFrameLayout& layout,
    const gfx::Rect& visible_rect,
    VideoFrame::StorageType storage_type) {
  auto fourcc =
      Fourcc::FromVideoPixelFormat(layout.format(), !layout.is_multi_planar());
  if (!fourcc) {
    DVLOG(1) << __func__ << " Failed to create Fourcc from video pixel format "
             << VideoPixelFormatToString(layout.format());
    return absl::nullopt;
  }
  return ImageProcessor::PortConfig(*fourcc, layout.coded_size(),
                                    layout.planes(), visible_rect,
                                    storage_type);
}

}  // namespace

struct WebOSVideoEncodeAccelerator::BitstreamBufferRef {
  BitstreamBufferRef(int32_t id, base::WritableSharedMemoryMapping shm_mapping)
      : id(id), shm_mapping(std::move(shm_mapping)) {}
  const int32_t id;
  base::WritableSharedMemoryMapping shm_mapping;
};

struct WebOSVideoEncodeAccelerator::InputRecord {
  InputRecord() = default;
  InputRecord(const InputRecord&) = default;
  ~InputRecord() = default;
  scoped_refptr<VideoFrame> frame;
  absl::optional<size_t> ip_output_buffer_index;
};

struct WebOSVideoEncodeAccelerator::InputFrameInfo {
  InputFrameInfo();
  InputFrameInfo(scoped_refptr<VideoFrame> frame, bool force_keyframe);
  InputFrameInfo(scoped_refptr<VideoFrame> frame,
                 bool force_keyframe,
                 size_t index);
  InputFrameInfo(const InputFrameInfo&) = default;
  ~InputFrameInfo() = default;
  scoped_refptr<VideoFrame> frame;
  bool force_keyframe;
  absl::optional<size_t> ip_output_buffer_index;
};

WebOSVideoEncodeAccelerator::InputFrameInfo::InputFrameInfo()
    : InputFrameInfo(nullptr, false) {}

WebOSVideoEncodeAccelerator::InputFrameInfo::InputFrameInfo(
    scoped_refptr<VideoFrame> frame,
    bool force_keyframe)
    : frame(frame), force_keyframe(force_keyframe) {}

WebOSVideoEncodeAccelerator::InputFrameInfo::InputFrameInfo(
    scoped_refptr<VideoFrame> frame,
    bool force_keyframe,
    size_t index)
    : frame(std::move(frame)),
      force_keyframe(force_keyframe),
      ip_output_buffer_index(index) {}

WebOSVideoEncodeAccelerator::WebOSVideoEncodeAccelerator()
    : child_task_runner_(base::SequencedTaskRunner::GetCurrentDefault()),
      encoder_task_runner_(base::ThreadPool::CreateSingleThreadTaskRunner(
          {base::WithBaseSyncPrimitives()},
          base::SingleThreadTaskRunnerThreadMode::DEDICATED)) {
  LOG(INFO) << __func__ << " Ctor";

  DCHECK_CALLED_ON_VALID_SEQUENCE(child_sequence_checker_);
  DETACH_FROM_SEQUENCE(encoder_sequence_checker_);
  weak_this_ = weak_this_factory_.GetWeakPtr();

  video_encoder_api_.reset(new mcil::VideoEncoderAPI(this));
}

WebOSVideoEncodeAccelerator::~WebOSVideoEncodeAccelerator() {
  LOG(INFO) << __func__ << " Dtor";
  DCHECK_CALLED_ON_VALID_SEQUENCE(encoder_sequence_checker_);
}

VideoEncodeAccelerator::SupportedProfiles
WebOSVideoEncodeAccelerator::GetSupportedProfiles() {
  mcil::SupportedProfiles supported_profiles =
      mcil::VideoEncoderAPI::GetSupportedProfiles();

  LOG(INFO) << __func__ << " supported profiles: " << supported_profiles.size();
  if (supported_profiles.empty()) {
    return SupportedProfiles();
  }

  SupportedProfiles profiles;
  for (const auto& supported_profile : supported_profiles) {
    SupportedRateControlMode rc_modes =
        static_cast<VideoEncodeAccelerator::SupportedRateControlMode>(
            supported_profile.rc_modes);
    SupportedProfile profile(VideoCodecProfileFrom(supported_profile.profile),
                             gfx::Size(supported_profile.max_resolution.width,
                                       supported_profile.max_resolution.height),
                             kMaxEncoderFramerate, 1, rc_modes);
    profile.min_resolution = gfx::Size(supported_profile.min_resolution.width,
                                       supported_profile.min_resolution.height);
    profile.max_resolution = gfx::Size(supported_profile.max_resolution.width,
                                       supported_profile.max_resolution.height);
    profiles.push_back(profile);
  }
  return profiles;
}

bool WebOSVideoEncodeAccelerator::Initialize(
    const Config& config,
    Client* client,
    std::unique_ptr<MediaLog> media_log) {
  VLOG(1) << __func__ << ": " << config.AsHumanReadableString();
  DCHECK_CALLED_ON_VALID_SEQUENCE(child_sequence_checker_);
  DCHECK_EQ(encoder_state_, mcil::kUninitialized);

  // V4L2VEA doesn't support temporal layers but we let it pass here to support
  // simulcast.
  if (config.HasSpatialLayer()) {
    LOG(ERROR) << __func__ << " Spatial layer encoding is supported";
    return false;
  }

  encoder_input_visible_rect_ = gfx::Rect(config.input_visible_size);

  client_ptr_factory_ = std::make_unique<base::WeakPtrFactory<Client>>(client);
  client_ = client_ptr_factory_->GetWeakPtr();

  bool result = false;
  base::WaitableEvent done;
  encoder_task_runner_->PostTask(
      FROM_HERE, base::BindOnce(&WebOSVideoEncodeAccelerator::InitializeTask,
                                weak_this_, config, &result, &done));
  done.Wait();
  return result;
}

void WebOSVideoEncodeAccelerator::Encode(scoped_refptr<VideoFrame> frame,
                                         bool force_keyframe) {
  VLOG(2) << __func__;
  DCHECK_CALLED_ON_VALID_SEQUENCE(child_sequence_checker_);

  encoder_task_runner_->PostTask(
      FROM_HERE, base::BindOnce(&WebOSVideoEncodeAccelerator::EncodeTask,
                                weak_this_, std::move(frame), force_keyframe));
}

void WebOSVideoEncodeAccelerator::UseOutputBitstreamBuffer(
    BitstreamBuffer buffer) {
  VLOG(2) << __func__ << " id=" << buffer.id();
  DCHECK_CALLED_ON_VALID_SEQUENCE(child_sequence_checker_);

  encoder_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(&WebOSVideoEncodeAccelerator::UseOutputBitstreamBufferTask,
                     weak_this_, std::move(buffer)));
}

void WebOSVideoEncodeAccelerator::RequestEncodingParametersChange(
    const Bitrate& bitrate,
    uint32_t framerate,
    const absl::optional<gfx::Size>& size) {
  VLOG(2) << __func__ << " bitrate=" << bitrate.target_bps()
          << ", framerate=" << framerate;
  DCHECK_CALLED_ON_VALID_SEQUENCE(child_sequence_checker_);

  encoder_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(
          &WebOSVideoEncodeAccelerator::RequestEncodingParametersChangeTask,
          weak_this_, bitrate, framerate, size));
}

void WebOSVideoEncodeAccelerator::Destroy() {
  VLOG(2) << __func__;
  DCHECK_CALLED_ON_VALID_SEQUENCE(child_sequence_checker_);

  client_ptr_factory_.reset();

  encoder_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(&WebOSVideoEncodeAccelerator::DestroyTask, weak_this_));
}

void WebOSVideoEncodeAccelerator::Flush(FlushCallback flush_callback) {
  VLOG(2) << __func__;
  DCHECK_CALLED_ON_VALID_SEQUENCE(child_sequence_checker_);

  encoder_task_runner_->PostTask(
      FROM_HERE, base::BindOnce(&WebOSVideoEncodeAccelerator::FlushTask,
                                weak_this_, std::move(flush_callback)));
}

bool WebOSVideoEncodeAccelerator::IsFlushSupported() {
  VLOG(2) << __func__;
  return video_encoder_api_->IsFlushSupported();
}

void WebOSVideoEncodeAccelerator::CreateInputBuffers(size_t count) {
  VLOG(2) << __func__;
  DCHECK_CALLED_ON_VALID_SEQUENCE(encoder_sequence_checker_);
  input_buffer_map_.resize(count);
}

void WebOSVideoEncodeAccelerator::DestroyInputBuffers() {
  VLOG(2) << __func__;
  DCHECK_CALLED_ON_VALID_SEQUENCE(encoder_sequence_checker_);
  input_buffer_map_.clear();
}

void WebOSVideoEncodeAccelerator::EnqueueInputBuffer(size_t buffer_index) {
  VLOG(2) << __func__ << " index=" << buffer_index;
  DCHECK_CALLED_ON_VALID_SEQUENCE(encoder_sequence_checker_);

  InputFrameInfo frame_info = encoder_input_queue_.front();
  InputRecord& input_record = input_buffer_map_[buffer_index];
  input_record.frame = std::move(frame_info.frame);
  input_record.ip_output_buffer_index = frame_info.ip_output_buffer_index;
  encoder_input_queue_.pop();
}

void WebOSVideoEncodeAccelerator::DequeueInputBuffer(size_t buffer_index) {
  VLOG(2) << __func__ << " index=" << buffer_index;
  DCHECK_CALLED_ON_VALID_SEQUENCE(encoder_sequence_checker_);

  InputRecord& input_record = input_buffer_map_[buffer_index];
  input_record.frame = nullptr;

  if (input_record.ip_output_buffer_index) {
    ReuseImageProcessorOutputBuffer(*input_record.ip_output_buffer_index);
  }
}

void WebOSVideoEncodeAccelerator::BitstreamBufferReady(
    mcil::ReadableBufferRef output_buffer) {
  VLOG(2) << __func__;
  DCHECK_CALLED_ON_VALID_SEQUENCE(encoder_sequence_checker_);
  output_buffer_queue_.push_back(std::move(output_buffer));
}

void WebOSVideoEncodeAccelerator::BitstreamBufferReady(const uint8_t* data,
                                                       size_t data_size,
                                                       uint64_t timestamp,
                                                       bool is_keyframe) {
  VLOG(2) << __func__ << " data_size: " << data_size;
  DCHECK_CALLED_ON_VALID_SEQUENCE(encoder_sequence_checker_);

  if (data_size > 0) {
    if (bitstream_buffer_pool_.empty()) {
      LOG(WARNING) << __func__ << " No free bitstream buffer, skip.";
      return;
    }

    auto buffer_ref = std::move(bitstream_buffer_pool_.back());
    auto buffer_id = buffer_ref->id;
    bitstream_buffer_pool_.pop_back();

    size_t output_data_size =
        CopyIntoOutputBuffer(data, data_size, std::move(buffer_ref));

    VLOG(2) << __func__ << " output_data_size: " << output_data_size;
    if (output_data_size >= 0) {
      child_task_runner_->PostTask(
          FROM_HERE,
          base::BindOnce(
              &Client::BitstreamBufferReady, client_, buffer_id,
              BitstreamBufferMetadata(output_data_size, is_keyframe,
                                      base::Microseconds(timestamp))));
    }
  }

  if (encoder_state_ == mcil::kFlushing) {
    VLOG(2) << __func__ << " Flush completed. Start the encoder again.";
    SetEncoderState(mcil::kEncoding);
    child_task_runner_->PostTask(
        FROM_HERE, base::BindOnce(std::move(flush_callback_), true));
  }
}

void WebOSVideoEncodeAccelerator::PumpBitstreamBuffers() {
  VLOG(2) << __func__;
  DCHECK_CALLED_ON_VALID_SEQUENCE(encoder_sequence_checker_);

  while (!output_buffer_queue_.empty()) {
    auto output_buf = std::move(output_buffer_queue_.front());
    output_buffer_queue_.pop_front();

    if (VLOG_IS_ON(1)) {
      frames_per_sec_++;
      base::TimeTicks curr_time = base::TimeTicks::Now();
      base::TimeDelta time_past = curr_time - old_time_;
      if (time_past >= base::Seconds(1)) {
        VLOG(1) << "--------------------------------------------------";
        VLOG(1) << __func__ << " encoded frames_per_sec: " << frames_per_sec_;
        VLOG(1) << "--------------------------------------------------";
        old_time_ = curr_time;
        frames_per_sec_ = 0;
      }
    }

    size_t bitstream_size = base::checked_cast<size_t>(
        output_buf->GetBytesUsed(0) - output_buf->GetDataOffset(0));
    if (bitstream_size > 0) {
      if (bitstream_buffer_pool_.empty()) {
        VLOG(4) << __func__ << "No free bitstream buffer, skip.";
        output_buffer_queue_.push_front(std::move(output_buf));
        break;
      }

      auto buffer_ref = std::move(bitstream_buffer_pool_.back());
      auto buffer_id = buffer_ref->id;
      bitstream_buffer_pool_.pop_back();

      size_t output_data_size = CopyIntoOutputBuffer(
          static_cast<const uint8_t*>(output_buf->GetPlaneBuffer(0)) +
              output_buf->GetDataOffset(0),
          bitstream_size, std::move(buffer_ref));

      VLOG(4) << __func__ << "returning buffer_id=" << buffer_id
              << ", size=" << output_data_size
              << ", key_frame=" << output_buf->IsKeyframe();
      child_task_runner_->PostTask(
          FROM_HERE,
          base::BindOnce(
              &Client::BitstreamBufferReady, client_, buffer_id,
              BitstreamBufferMetadata(
                  output_data_size, output_buf->IsKeyframe(),
                  base::Microseconds(output_buf->GetTimeStamp().tv_usec +
                                     output_buf->GetTimeStamp().tv_sec *
                                         base::Time::kMicrosecondsPerSecond))));
    }

    if ((encoder_state_ == mcil::kFlushing) && output_buf->IsLast()) {
      VLOG(3) << __func__ << "Flush completed. Start the encoder again.";
      SetEncoderState(mcil::kEncoding);

      child_task_runner_->PostTask(
          FROM_HERE, base::BindOnce(std::move(flush_callback_), true));

      video_encoder_api_->SendStartCommand(true);
    }
  }

  if (video_encoder_api_->GetFreeBuffersCount(mcil::OUTPUT_QUEUE) > 0) {
    encoder_task_runner_->PostTask(
        FROM_HERE,
        base::BindOnce(&WebOSVideoEncodeAccelerator::Enqueue, weak_this_));
  }
}

uint8_t WebOSVideoEncodeAccelerator::GetH264LevelLimit(
    const mcil::EncoderConfig* config) {
  VLOG(2) << __func__;
  constexpr size_t kH264MacroblockSizeInPixels = 16;
  // TODO::check size_t(config->width) is correct?
  const uint32_t mb_width =
      base::bits::AlignUp(size_t(config->width), kH264MacroblockSizeInPixels) /
      kH264MacroblockSizeInPixels;
  const uint32_t mb_height =
      base::bits::AlignUp(size_t(config->height), kH264MacroblockSizeInPixels) /
      kH264MacroblockSizeInPixels;
  const uint32_t framesize_in_mbs = mb_width * mb_height;
  uint8_t h264_level = config->h264OutputLevel;

  // Check whether the h264 level is valid.
  if (!CheckH264LevelLimits(VideoCodecProfileFrom(config->profile), h264_level,
                            config->bitRate, config->frameRate,
                            framesize_in_mbs)) {
    absl::optional<uint8_t> valid_level = FindValidH264Level(
        VideoCodecProfileFrom(config->profile), config->bitRate,
        config->frameRate, framesize_in_mbs);
    if (!valid_level) {
      gfx::Size input_visible_size(config->width, config->height);
      LOG(ERROR) << __func__ << " Could not find a valid h264 level for"
                 << " profile=" << VideoCodecProfileFrom(config->profile)
                 << " bitrate=" << config->bitRate
                 << " framerate=" << config->frameRate
                 << " size=" << input_visible_size.ToString();
      NOTIFY_ERROR(EncoderStatus::Codes::kEncoderUnsupportedConfig);
      return h264_level;
    }

    h264_level = *valid_level;
  }
  return h264_level;
}

void WebOSVideoEncodeAccelerator::StopDevicePoll() {
  VLOG(2) << __func__;
  DCHECK_CALLED_ON_VALID_SEQUENCE(encoder_sequence_checker_);

  // Reset all our accounting info.
  while (!encoder_input_queue_.empty()) {
    encoder_input_queue_.pop();
  }

  for (auto& input_record : input_buffer_map_) {
    input_record.frame = nullptr;
  }

  bitstream_buffer_pool_.clear();
}

void WebOSVideoEncodeAccelerator::NotifyFlushIfNeeded(bool flush) {
  VLOG(2) << __func__;
  DCHECK_CALLED_ON_VALID_SEQUENCE(encoder_sequence_checker_);

  child_task_runner_->PostTask(
      FROM_HERE, base::BindOnce(std::move(flush_callback_), flush));
}

void WebOSVideoEncodeAccelerator::NotifyEncodeBufferTask() {
  VLOG(2) << __func__;
  encoder_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(&WebOSVideoEncodeAccelerator::RunEncodeBufferTask,
                     weak_this_));
}

void WebOSVideoEncodeAccelerator::NotifyEncoderError(
    mcil::EncoderError error_code) {
  LOG(ERROR) << __func__ << " platform error_code: " << error_code;
  DCHECK_CALLED_ON_VALID_SEQUENCE(encoder_sequence_checker_);
  NOTIFY_ERROR(error_code == mcil::kIllegalStateError
                   ? EncoderStatus::Codes::kEncoderIllegalState
                   : EncoderStatus::Codes::kEncoderHardwareDriverError);
}

void WebOSVideoEncodeAccelerator::NotifyEncoderState(mcil::CodecState state) {
  VLOG(2) << __func__ << " state=" << state;
  DCHECK_CALLED_ON_VALID_SEQUENCE(encoder_sequence_checker_);
  SetEncoderState(state);
}

void WebOSVideoEncodeAccelerator::InitializeTask(const Config& config,
                                                 bool* result,
                                                 base::WaitableEvent* done) {
  LOG(INFO) << __func__ << ": " << config.AsHumanReadableString();
  DCHECK_CALLED_ON_VALID_SEQUENCE(encoder_sequence_checker_);

  // Signal the event when leaving the method.
  base::ScopedClosureRunner signal_event(
      base::BindOnce(&base::WaitableEvent::Signal, base::Unretained(done)));
  *result = false;

  input_frame_size_ = VideoFrame::DetermineAlignedSize(
      config.input_format, encoder_input_visible_rect_.size());
  output_buffer_byte_size_ =
      GetEncodeBitstreamBufferSize(encoder_input_visible_rect_.size());

  VideoCodec video_codec = VideoCodecProfileToVideoCodec(config.output_profile);
  mcil::EncoderConfig encoder_config;
  encoder_config.pixelFormat = MCILVideoPixelFormatFrom(config.input_format);
  encoder_config.profile = MCILVideoCodecProfileFrom(config.output_profile);
  encoder_config.width = encoder_input_visible_rect_.width();
  encoder_config.height = encoder_input_visible_rect_.height();
  encoder_config.bitRate = config.bitrate.target_bps();
  encoder_config.frameRate = config.framerate;
  encoder_config.outputBufferSize = output_buffer_byte_size_;
  encoder_config.h264OutputLevel =
      config.h264_output_level.value_or(H264SPS::kLevelIDC4p0);
  encoder_config.gopLength = config.gop_length.value_or(0);

  mcil::EncoderClientConfig client_config;
  client_config.output_buffer_byte_size = output_buffer_byte_size_;
  if (!video_encoder_api_->Initialize(&encoder_config, &client_config)) {
    LOG(ERROR) << __func__ << " Error initializing encoder.";
    NOTIFY_ERROR(EncoderStatus::Codes::kEncoderInitializationError);
    return;
  }

  output_buffer_byte_size_ = client_config.output_buffer_byte_size;
  should_control_buffer_feed_ = client_config.should_control_buffer_feed;
  inject_sps_and_pps_ = client_config.should_inject_sps_and_pps;
  input_frame_size_ = gfx::Size(client_config.input_frame_size.width,
                                client_config.input_frame_size.height);

  device_input_layout_ =
      VideoFrameLayoutFrom(video_encoder_api_->GetDeviceInputFrame());
  if (device_input_layout_.has_value() &&
      config.input_format != device_input_layout_->format()) {
    VLOG(1) << __func__ << " Input format: " << config.input_format
            << " is not supported by the HW. Will try to convert to "
            << device_input_layout_->format();
    auto input_layout = VideoFrameLayout::CreateMultiPlanar(
        config.input_format, encoder_input_visible_rect_.size(),
        std::vector<ColorPlaneLayout>(
            VideoFrame::NumPlanes(config.input_format)));
    if (!input_layout) {
      LOG(ERROR) << __func__ << " Invalid image processor input layout";
      return;
    }

    if (!CreateImageProcessor(*input_layout, device_input_layout_->format(),
                              device_input_layout_->coded_size(),
                              encoder_input_visible_rect_,
                              encoder_input_visible_rect_)) {
      LOG(ERROR) << __func__ << " Failed to create image processor";
      return;
    }

    const mcil::Size output_buf_size(
        image_processor_->output_config().planes[0].stride,
        image_processor_->output_config().size.height());
    mcil::VideoPixelFormat format =
        MCILVideoPixelFormatFrom(device_input_layout_->format());

    if (!video_encoder_api_->NegotiateInputFormat(format, output_buf_size)) {
      gfx::Size op_buffer_size =
          gfx::Size(output_buf_size.width, output_buf_size.height);
      LOG(ERROR) << __func__
                 << " Failed to reconfigure v4l2 encoder driver with the "
                 << "ImageProcessor output buffer: "
                 << op_buffer_size.ToString();
      return;
    }
  }

  SetEncoderState(mcil::kInitialized);
  LOG(INFO) << __func__ << " image_processor[" << image_processor_.get() << "]";

  if (image_processor_.get()) {
    input_frame_size_ = image_processor_->input_config().size;
  }

  child_task_runner_->PostTask(
      FROM_HERE, base::BindOnce(&Client::RequireBitstreamBuffers, client_,
                                kInputBufferCount, input_frame_size_,
                                output_buffer_byte_size_));

  // Notify VideoEncoderInfo after initialization.
  VideoEncoderInfo encoder_info;
  encoder_info.implementation_name = "WebOSVideoEncodeAccelerator";
  encoder_info.has_trusted_rate_controller = true;
  encoder_info.is_hardware_accelerated = true;
  encoder_info.supports_native_handle = true;
  encoder_info.supports_simulcast = false;

  constexpr uint8_t kFullFramerate = 255;
  encoder_info.fps_allocation[0] = {kFullFramerate};
  child_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(&Client::NotifyEncoderInfoChange, client_, encoder_info));
  *result = true;
  LOG(INFO) << __func__ << " : " << (*result ? " SUCCESS" : "FAILED");
}

void WebOSVideoEncodeAccelerator::EncodeTask(
    scoped_refptr<VideoFrame> video_frame,
    bool force_keyframe) {
  VLOG(2) << __func__;
  DCHECK_CALLED_ON_VALID_SEQUENCE(encoder_sequence_checker_);
  DCHECK_NE(encoder_state_, mcil::kUninitialized);

  if (encoder_state_ == mcil::kEncoderError) {
    LOG(WARNING) << __func__ << " early out: kError state";
    return;
  }

  if (should_control_buffer_feed_) {
    if (video_frame) {
      encoder_input_queue_.emplace(std::move(video_frame), force_keyframe);
      encoder_task_runner_->PostTask(
          FROM_HERE,
          base::BindOnce(
              &WebOSVideoEncodeAccelerator::FeedBufferOnEncoderThread,
              weak_this_));
    }
    return;
  }

  if (image_processor_) {
    image_processor_input_queue_.emplace(std::move(video_frame),
                                         force_keyframe);
    InputImageProcessorTask();
  } else {
    mcil::scoped_refptr<mcil::VideoFrame> mcil_frame = ToMcilFrame(video_frame);
    encoder_input_queue_.emplace(std::move(video_frame), force_keyframe);
    video_encoder_api_->EncodeFrame(mcil_frame, force_keyframe);
  }
}

void WebOSVideoEncodeAccelerator::UseOutputBitstreamBufferTask(
    BitstreamBuffer buffer) {
  VLOG(2) << __func__ << ": id=" << buffer.id();
  DCHECK_CALLED_ON_VALID_SEQUENCE(encoder_sequence_checker_);

  if (buffer.size() < output_buffer_byte_size_) {
    LOG(ERROR) << __func__ << " Invalid buffer size.";
    NOTIFY_ERROR(EncoderStatus::Codes::kInvalidOutputBuffer);
    return;
  }

  base::UnsafeSharedMemoryRegion shm_region = buffer.TakeRegion();
  base::WritableSharedMemoryMapping shm_mapping =
      shm_region.MapAt(buffer.offset(), buffer.size());

  if (!shm_mapping.IsValid()) {
    NOTIFY_ERROR(EncoderStatus::Codes::kSystemAPICallError);
    return;
  }

  bitstream_buffer_pool_.push_back(std::make_unique<BitstreamBufferRef>(
      buffer.id(), std::move(shm_mapping)));
  PumpBitstreamBuffers();

  if (encoder_state_ == mcil::kInitialized) {
    if (!video_encoder_api_->StartDevicePoll()) {
      return;
    }
    SetEncoderState(mcil::kEncoding);
  }
}

void WebOSVideoEncodeAccelerator::RequestEncodingParametersChangeTask(
    const Bitrate& bitrate,
    uint32_t framerate,
    const absl::optional<gfx::Size>& size) {
  VLOG(2) << __func__ << " bitrate=" << bitrate.target_bps()
          << ", framerate=" << framerate;
  DCHECK_CALLED_ON_VALID_SEQUENCE(encoder_sequence_checker_);

  if (!video_encoder_api_) {
    LOG(ERROR) << __func__ << " Error platform encoder instance not created.";
    return;
  }

  // If this is changed to use variable bitrate encoding, change this to check
  // that the mode matches the current mode.
  if (bitrate.mode() != Bitrate::Mode::kConstant) {
    return;
  }

  if (current_bitrate_ == bitrate.target_bps() &&
      current_framerate_ == framerate) {
    return;
  }
  if (bitrate.target_bps() == 0 || framerate == 0) {
    return;
  }

  VLOG(2) << __func__ << " bitrate=" << bitrate.ToString()
          << ", framerate=" << framerate;

  if (video_encoder_api_->UpdateEncodingParams(bitrate.target_bps(),
                                               framerate)) {
    current_bitrate_ = bitrate.target_bps();
    current_framerate_ = framerate;
  }
}

void WebOSVideoEncodeAccelerator::DestroyTask() {
  VLOG(2) << __func__;
  DCHECK_CALLED_ON_VALID_SEQUENCE(encoder_sequence_checker_);

  weak_this_factory_.InvalidateWeakPtrs();

  // If a flush is pending, notify client that it did not finish.
  if (flush_callback_) {
    child_task_runner_->PostTask(
        FROM_HERE, base::BindOnce(std::move(flush_callback_), false));
  }

  video_encoder_api_->Destroy();
  delete this;
}

void WebOSVideoEncodeAccelerator::FlushTask(FlushCallback flush_callback) {
  VLOG(2) << __func__;
  DCHECK_CALLED_ON_VALID_SEQUENCE(encoder_sequence_checker_);

  if (flush_callback_ || encoder_state_ != mcil::kEncoding) {
    LOG(ERROR) << __func__ << " Flush failed: there is a pending flush, "
               << "or VideoEncodeAccelerator is not in kEncoding state";
    NOTIFY_ERROR(EncoderStatus::Codes::kEncoderIllegalState);
    child_task_runner_->PostTask(
        FROM_HERE, base::BindOnce(std::move(flush_callback), false));
    return;
  }
  flush_callback_ = std::move(flush_callback);

  EncodeTask(nullptr, false);
}

void WebOSVideoEncodeAccelerator::NotifyError(EncoderStatus status) {
  LOG(ERROR) << __func__ << ": error=" << status.message();
  DCHECK(child_task_runner_);

  if (child_task_runner_->RunsTasksInCurrentSequence()) {
    if (client_) {
      client_->NotifyErrorStatus(status);
      client_ptr_factory_.reset();
    }
    return;
  }

  child_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(&Client::NotifyErrorStatus, client_, std::move(status)));
}

void WebOSVideoEncodeAccelerator::SetErrorState(EncoderStatus status) {
  LOG(ERROR) << __func__ << ": status=" << status.message();

  if (!encoder_task_runner_->RunsTasksInCurrentSequence()) {
    encoder_task_runner_->PostTask(
        FROM_HERE, base::BindOnce(&WebOSVideoEncodeAccelerator::SetErrorState,
                                  weak_this_, status));
    return;
  }

  if (encoder_state_ != mcil::kEncoderError &&
      encoder_state_ != mcil::kUninitialized) {
    NotifyError(status);
  }

  SetEncoderState(mcil::kEncoderError);
}

void WebOSVideoEncodeAccelerator::SetEncoderState(mcil::CodecState state) {
  VLOG(2) << __func__ << " encoder_state[ " << encoder_state_ << " -> " << state
          << " ]";

  if (encoder_state_ == state) {
    return;
  }

  encoder_state_ = state;
  video_encoder_api_->SetEncoderState(encoder_state_);
}

void WebOSVideoEncodeAccelerator::Enqueue() {
  VLOG(2) << __func__;
  DCHECK_CALLED_ON_VALID_SEQUENCE(encoder_sequence_checker_);
  video_encoder_api_->EnqueueBuffers();
}

void WebOSVideoEncodeAccelerator::RunEncodeBufferTask() {
  VLOG(2) << __func__;
  DCHECK_CALLED_ON_VALID_SEQUENCE(encoder_sequence_checker_);
  video_encoder_api_->RunEncodeBufferTask();
}

size_t WebOSVideoEncodeAccelerator::CopyIntoOutputBuffer(
    const uint8_t* bitstream_data,
    size_t bitstream_size,
    std::unique_ptr<BitstreamBufferRef> buffer_ref) {
  VLOG(2) << __func__ << " bitstream_data: " << bitstream_data
          << " bitstream_size; " << bitstream_size;
  DCHECK_CALLED_ON_VALID_SEQUENCE(encoder_sequence_checker_);

  uint8_t* dst_ptr =
      static_cast<uint8_t*>(buffer_ref->shm_mapping.GetMemoryAs<uint8_t>());
  size_t remaining_dst_size = buffer_ref->shm_mapping.size();

  if (!inject_sps_and_pps_) {
    if (bitstream_size <= remaining_dst_size) {
      memcpy(dst_ptr, bitstream_data, bitstream_size);
      return bitstream_size;
    } else {
      VLOG(1) << __func__ << "Output data did not fit in the BitstreamBuffer";
      return 0;
    }
  }

  H264Parser parser;
  parser.SetStream(bitstream_data, bitstream_size);
  media::H264NALU nalu;

  bool inserted_sps = false;
  bool inserted_pps = false;
  while (parser.AdvanceToNextNALU(&nalu) == H264Parser::kOk) {
    // nalu.size is always without the start code, regardless of the NALU type.
    if (nalu.size + kH264StartCodeSize > remaining_dst_size) {
      LOG(WARNING) << __func__ << " data did not fit in the BitstreamBuffer";
      break;
    }

    switch (nalu.nal_unit_type) {
      case media::H264NALU::kSPS:
        cached_sps_.resize(nalu.size);
        memcpy(cached_sps_.data(), nalu.data, nalu.size);
        cached_h264_header_size_ =
            cached_sps_.size() + cached_pps_.size() + 2 * kH264StartCodeSize;
        inserted_sps = true;
        break;

      case media::H264NALU::kPPS:
        cached_pps_.resize(nalu.size);
        memcpy(cached_pps_.data(), nalu.data, nalu.size);
        cached_h264_header_size_ =
            cached_sps_.size() + cached_pps_.size() + 2 * kH264StartCodeSize;
        inserted_pps = true;
        break;

      case media::H264NALU::kIDRSlice:
        if (inserted_sps && inserted_pps) {
          // Already inserted SPS and PPS. No need to inject.
          break;
        }
        // Only inject if we have both headers cached, and enough space for both
        // the headers and the NALU itself.
        if (cached_sps_.empty() || cached_pps_.empty()) {
          VLOG(2) << __func__ << " Cannot inject IDR slice without SPS and PPS";
          break;
        }
        if (cached_h264_header_size_ + nalu.size + kH264StartCodeSize >
            remaining_dst_size) {
          VLOG(2) << __func__
                  << " Not enough space to inject a stream header before IDR";
          break;
        }

        if (!inserted_sps) {
          CopyNALUPrependingStartCode(cached_sps_.data(), cached_sps_.size(),
                                      &dst_ptr, &remaining_dst_size);
        }
        if (!inserted_pps) {
          CopyNALUPrependingStartCode(cached_pps_.data(), cached_pps_.size(),
                                      &dst_ptr, &remaining_dst_size);
        }
        VLOG(2) << __func__ << " Stream header injected before IDR";
        break;
    }

    CopyNALUPrependingStartCode(nalu.data, nalu.size, &dst_ptr,
                                &remaining_dst_size);
  }

  return buffer_ref->shm_mapping.size() - remaining_dst_size;
}

void WebOSVideoEncodeAccelerator::FeedBufferOnEncoderThread() {
  VLOG(2) << __func__;

  if (!video_encoder_api_) {
    LOG(ERROR) << __func__ << " Error encoder client not created.";
    return;
  }

  while (!encoder_input_queue_.empty()) {
    InputFrameInfo frame_info = encoder_input_queue_.front();
    scoped_refptr<VideoFrame> frame = frame_info.frame;

    uint64_t buffer_ts = frame->timestamp().InMicroseconds();
    size_t y_size =
        frame->visible_rect().width() * frame->visible_rect().height();
    size_t uv_size = ((frame->visible_rect().width() + 1) >> 1) *
                     ((frame->visible_rect().height() + 1) >> 1);
    if (!video_encoder_api_->EncodeBuffer(
            frame->visible_data(media::VideoFrame::Plane::kY), y_size,
            frame->visible_data(media::VideoFrame::Plane::kU), uv_size,
            frame->visible_data(media::VideoFrame::Plane::kV), uv_size, buffer_ts,
            frame_info.force_keyframe)) {
      LOG(ERROR) << __func__ << " Error feeding buffer.";
      return;
    }
    encoder_input_queue_.pop();
  }
}

bool WebOSVideoEncodeAccelerator::CreateImageProcessor(
    const VideoFrameLayout& input_layout,
    const VideoPixelFormat output_format,
    const gfx::Size& output_size,
    const gfx::Rect& input_visible_rect,
    const gfx::Rect& output_visible_rect) {
  VLOG(1) << __func__;
  DCHECK_CALLED_ON_VALID_SEQUENCE(encoder_sequence_checker_);

  auto ip_input_layout = AsMultiPlanarLayout(input_layout);
  if (!ip_input_layout) {
    VLOG(1) << __func__
            << " Failed to get multi-planar input layout, input_layout="
            << input_layout;
    return false;
  }

  auto input_config = VideoFrameLayoutToPortConfig(
      *ip_input_layout, input_visible_rect, VideoFrame::STORAGE_SHMEM);
  if (!input_config) {
    VLOG(1) << __func__ << " Failed to create ImageProcessor input config";
    return false;
  }
  VLOG(1) << " : input_config=[" << input_config->ToString() << "]";

  std::vector<ColorPlaneLayout> planes = device_input_layout_->planes();
  for (size_t i = 0; i < device_input_layout_->num_planes(); i++) {
    if (i == 0) {
      planes[i].stride = planes[0].stride;
      planes[i].offset = 0;
      planes[i].size =
          planes[0].stride * device_input_layout_->coded_size().height();
    } else {
      planes[i].stride = planes[0].stride;
      planes[i].offset = planes[0].offset;
      planes[i].size =
          planes[0].stride * (device_input_layout_->coded_size().height() / 2);
    }
  }
  auto ip_output_layout = VideoFrameLayout::CreateWithPlanes(
      device_input_layout_->format(), device_input_layout_->coded_size(),
      planes);
  if (!ip_output_layout) {
    VLOG(1) << __func__ << " Failed to get Output VideoFrameLayout";
    return false;
  }

  auto output_config = VideoFrameLayoutToPortConfig(
      *ip_output_layout, output_visible_rect, VideoFrame::STORAGE_OWNED_MEMORY);
  if (!output_config) {
    VLOG(1) << __func__ << " Failed to create ImageProcessor output config";
    return false;
  }

  VLOG(1) << " : output_config=[" << output_config->ToString() << "]";

  image_processor_ = ImageProcessorFactory::Create(
      *input_config, *output_config, kImageProcBufferCount,
      base::BindRepeating(&WebOSVideoEncodeAccelerator::ImageProcessorError,
                          weak_this_),
      encoder_task_runner_);

  if (!image_processor_) {
    VLOG(1) << __func__ << " Failed initializing image processor";
    return false;
  }
  num_frames_in_image_processor_ = 0;

  const auto& ip_output_size = image_processor_->output_config().size;
  if (ip_output_size.width() != ip_output_layout->coded_size().width() ||
      ip_output_size.height() < ip_output_layout->coded_size().height()) {
    VLOG(1) << __func__ << " Invalid image processor output coded size "
            << ip_output_size.ToString() << ", expected output coded size is "
            << ip_output_layout->coded_size().ToString();
    return false;
  }

  free_image_processor_output_buffer_indices_.resize(kImageProcBufferCount);
  std::iota(free_image_processor_output_buffer_indices_.begin(),
            free_image_processor_output_buffer_indices_.end(), 0);
  return AllocateImageProcessorOutputBuffers(ip_output_layout,
                                             kImageProcBufferCount);
}

bool WebOSVideoEncodeAccelerator::AllocateImageProcessorOutputBuffers(
    absl::optional<VideoFrameLayout> output_layout,
    size_t count) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(encoder_sequence_checker_);
  DCHECK(image_processor_);
  DCHECK_EQ(image_processor_->output_mode(),
            ImageProcessor::OutputMode::IMPORT);

  image_processor_output_buffers_.clear();
  image_processor_output_buffers_.resize(count);
  const ImageProcessor::PortConfig& output_config =
      image_processor_->output_config();
  for (size_t i = 0; i < count; i++) {
    switch (output_config.storage_type) {
      case VideoFrame::STORAGE_OWNED_MEMORY:
        image_processor_output_buffers_[i] = VideoFrame::CreateFrameWithLayout(
            *output_layout, gfx::Rect(output_layout->coded_size()),
            output_layout->coded_size(), base::TimeDelta(), false);
        break;
      default:
        VLOG(1) << __func__
                << " Unsupported output storage type of image processor: "
                << output_config.storage_type;
        return false;
    }
    if (!image_processor_output_buffers_[i]) {
      VLOG(1) << __func__ << " Failed to create VideoFrame";
      return false;
    }
  }
  return true;
}

void WebOSVideoEncodeAccelerator::InputImageProcessorTask() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(encoder_sequence_checker_);
  VLOG(1) << __func__;

  MaybeFlushImageProcessor();

  if (free_image_processor_output_buffer_indices_.empty()) {
    return;
  }

  if (image_processor_input_queue_.empty()) {
    return;
  }

  if (!image_processor_input_queue_.front().frame) {
    return;
  }

  const size_t output_buffer_index =
      free_image_processor_output_buffer_indices_.back();
  free_image_processor_output_buffer_indices_.pop_back();

  InputFrameInfo frame_info = std::move(image_processor_input_queue_.front());
  image_processor_input_queue_.pop();
  auto frame = std::move(frame_info.frame);
  const bool force_keyframe = frame_info.force_keyframe;
  auto timestamp = frame->timestamp();

  auto output_frame = image_processor_output_buffers_[output_buffer_index];

  if (!image_processor_->Process(
          std::move(frame), std::move(output_frame),
          base::BindOnce(&WebOSVideoEncodeAccelerator::FrameProcessed,
                         weak_this_, force_keyframe, timestamp,
                         output_buffer_index))) {
    LOG(ERROR) << __func__ << " Image processor Process error";
    NOTIFY_ERROR(EncoderStatus::Codes::kFormatConversionError);
  }

  num_frames_in_image_processor_++;
}

void WebOSVideoEncodeAccelerator::MaybeFlushImageProcessor() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(encoder_sequence_checker_);
  DCHECK(image_processor_);
  if (image_processor_input_queue_.size() == 1 &&
      !image_processor_input_queue_.front().frame &&
      num_frames_in_image_processor_ == 0) {
    DVLOG(3) << __func__ << " All frames to be flush have been processed by "
             << "|image_processor_|. Move the flush request to the encoder";
    image_processor_input_queue_.pop();
    encoder_input_queue_.emplace(nullptr, false);
    Enqueue();
  }
}

void WebOSVideoEncodeAccelerator::ReuseImageProcessorOutputBuffer(
    size_t output_buffer_index) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(encoder_sequence_checker_);
  VLOG(1) << __func__ << " output_buffer_index=" << output_buffer_index;

  free_image_processor_output_buffer_indices_.push_back(output_buffer_index);
  InputImageProcessorTask();
}

void WebOSVideoEncodeAccelerator::FrameProcessed(
    bool force_keyframe,
    base::TimeDelta timestamp,
    size_t output_buffer_index,
    scoped_refptr<VideoFrame> video_frame) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(encoder_sequence_checker_);
  VLOG(1) << __func__ << " force_keyframe= " << force_keyframe
          << ", output_buffer_index= " << output_buffer_index;
  DCHECK_GE(output_buffer_index, 0u);

  if (!encoder_task_runner_->RunsTasksInCurrentSequence()) {
    encoder_task_runner_->PostTask(
        FROM_HERE, base::BindOnce(&WebOSVideoEncodeAccelerator::FrameProcessed,
                                  weak_this_, force_keyframe, timestamp,
                                  output_buffer_index, video_frame));
    return;
  }

  mcil::scoped_refptr<mcil::VideoFrame> mcil_frame = ToMcilFrame(video_frame);
  encoder_input_queue_.emplace(std::move(video_frame), force_keyframe,
                               output_buffer_index);
  video_encoder_api_->EncodeFrame(mcil_frame, force_keyframe);

  CHECK_GT(num_frames_in_image_processor_, 0u);
  num_frames_in_image_processor_--;
  MaybeFlushImageProcessor();
}

void WebOSVideoEncodeAccelerator::ImageProcessorError() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(encoder_sequence_checker_);
  LOG(ERROR) << __func__ << " Image processor error";
  NOTIFY_ERROR(EncoderStatus::Codes::kFormatConversionError);
}

mcil::scoped_refptr<mcil::VideoFrame> WebOSVideoEncodeAccelerator::ToMcilFrame(
    scoped_refptr<VideoFrame> video_frame) {
  VLOG(2) << __func__ << " video_frame= " << video_frame;

  if (!video_frame) {
    return nullptr;
  }

  mcil::Size coded_size(video_frame->coded_size().width(),
                        video_frame->coded_size().height());

  mcil::scoped_refptr<mcil::VideoFrame> mcil_frame =
      mcil::VideoFrame::Create(coded_size);
  mcil_frame->timestamp.tv_sec =
      static_cast<time_t>(video_frame->timestamp().InSeconds());
  mcil_frame->timestamp.tv_usec =
      video_frame->timestamp().InMicroseconds() -
      video_frame->timestamp().InSeconds() * base::Time::kMicrosecondsPerSecond;
  mcil_frame->format = MCILVideoPixelFormatFrom(video_frame->format());

  size_t num_planes = VideoFrame::NumPlanes(video_frame->format());
  for (size_t i = 0; i < num_planes; ++i) {
    mcil_frame->data[i] = (uint8_t*)video_frame->data(i);
  }
  return mcil_frame;
}

}  // namespace media
