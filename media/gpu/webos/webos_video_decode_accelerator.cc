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

#include "media/gpu/webos/webos_video_decode_accelerator.h"

#include <mcil/video_decoder_api.h>

#include "base/check_op.h"
#include "base/location.h"
#include "build/build_config.h"
#include "media/gpu/webos/webos_codec_utils.h"
#include "media/gpu/webos/webos_vda_utils.h"
#include "ui/gl/gl_context.h"
#include "ui/gl/gl_display.h"
#include "ui/gl/scoped_binders.h"

#define NOTIFY_ERROR(x) \
  do {                  \
    SetErrorState(x);   \
  } while (0)

namespace media {

// static
VideoDecodeAccelerator::SupportedProfiles
WebOSVideoDecodeAccelerator::GetSupportedProfiles() {
  mcil::SupportedProfiles supported_profiles =
      mcil::VideoDecoderAPI::GetSupportedProfiles();

  LOG(INFO) << __func__ << " supported profiles: " << supported_profiles.size();
  if (supported_profiles.empty())
    return SupportedProfiles();

  SupportedProfiles profiles;
  for (const auto& supported_profile : supported_profiles) {
    SupportedProfile profile;
    profile.profile = VideoCodecProfileFrom(supported_profile.profile);
    profile.min_resolution = gfx::Size(supported_profile.min_resolution.width,
                                       supported_profile.min_resolution.height);
    profile.max_resolution = gfx::Size(supported_profile.max_resolution.width,
                                       supported_profile.max_resolution.height);
    profiles.push_back(profile);
  }
  return profiles;
}

struct WebOSVideoDecodeAccelerator::BitstreamBufferRef {
  BitstreamBufferRef(
      base::WeakPtr<Client>& client,
      scoped_refptr<base::SequencedTaskRunner>& client_task_runner,
      scoped_refptr<DecoderBuffer> buffer,
      int32_t input_id);
  ~BitstreamBufferRef();

  const base::WeakPtr<Client> client;
  const scoped_refptr<base::SequencedTaskRunner> client_task_runner;
  scoped_refptr<DecoderBuffer> buffer;
  size_t bytes_used;
  const int32_t input_id;
};

WebOSVideoDecodeAccelerator::BitstreamBufferRef::BitstreamBufferRef(
    base::WeakPtr<Client>& client,
    scoped_refptr<base::SequencedTaskRunner>& client_task_runner,
    scoped_refptr<DecoderBuffer> buffer,
    int32_t input_id)
    : client(client),
      client_task_runner(client_task_runner),
      buffer(std::move(buffer)),
      bytes_used(0),
      input_id(input_id) {}

WebOSVideoDecodeAccelerator::BitstreamBufferRef::~BitstreamBufferRef() {
  if (input_id >= 0) {
    client_task_runner->PostTask(
        FROM_HERE,
        base::BindOnce(&Client::NotifyEndOfBitstreamBuffer, client, input_id));
  }
}

struct WebOSVideoDecodeAccelerator::OutputRecord {
  OutputRecord();
  OutputRecord(OutputRecord&&);
  ~OutputRecord();
  EGLImageKHR egl_image;
  int32_t picture_id;
  GLuint texture_id;
  bool cleared;
  scoped_refptr<VideoFrame> output_frame;
};

WebOSVideoDecodeAccelerator::OutputRecord::OutputRecord()
    : egl_image(EGL_NO_IMAGE_KHR),
      picture_id(-1),
      texture_id(0),
      cleared(false) {}

WebOSVideoDecodeAccelerator::OutputRecord::OutputRecord(OutputRecord&&) =
    default;

WebOSVideoDecodeAccelerator::OutputRecord::~OutputRecord() {}

WebOSVideoDecodeAccelerator::WebOSVideoDecodeAccelerator(
    EGLDisplay egl_display,
    const GetGLContextCallback& get_gl_context_cb,
    const MakeGLContextCurrentCallback& make_context_current_cb)
    : egl_display_(egl_display),
      get_gl_context_cb_(get_gl_context_cb),
      make_context_current_cb_(make_context_current_cb),
      decoder_thread_("WebOSDecoderThread"),
      decoder_state_(mcil::kUninitialized),
      child_task_runner_(base::SequencedTaskRunner::GetCurrentDefault()),
      output_mode_(Config::OutputMode::kAllocate),
      weak_this_factory_(this) {
  weak_this_ = weak_this_factory_.GetWeakPtr();
  LOG(INFO) << __func__ << " Ctor";
  video_decoder_api_.reset(new mcil::VideoDecoderAPI(this));
  webos_video_utils_ = new vda::WebOSVideoUtils();
}

WebOSVideoDecodeAccelerator::~WebOSVideoDecodeAccelerator() {
  LOG(INFO) << __func__ << " Dtor";
  DCHECK(!decoder_thread_.IsRunning());
}

bool WebOSVideoDecodeAccelerator::Initialize(const Config& config,
                                             Client* client) {
  LOG(INFO) << __func__ << " profile: " << config.profile
            << " config: " << config.AsHumanReadableString();

  if (config.is_encrypted()) {
    NOTREACHED() << " Encrypted streams are not supported for this VDA";
    return false;
  }

  if (config.output_mode != Config::OutputMode::kAllocate &&
      config.output_mode != Config::OutputMode::kImport) {
    NOTREACHED() << " Only ALLOCATE and IMPORT OutputModes are supported";
    return false;
  }

  client_ptr_factory_.reset(new base::WeakPtrFactory<Client>(client));
  client_ = client_ptr_factory_->GetWeakPtr();

  if (!decode_task_runner_) {
    decode_task_runner_ = child_task_runner_;
    DCHECK(!decode_client_);
    decode_client_ = client_;
  }

  if (make_context_current_cb_) {
    if (egl_display_ == EGL_NO_DISPLAY) {
      LOG(ERROR) << __func__ << " could not get EGLDisplay";
      return false;
    }

    if (!make_context_current_cb_.Run()) {
      LOG(ERROR) << __func__ << " could not make context current";
      return false;
    }

#if defined(ARCH_CPU_ARMEL)
    gl::GLDisplayEGL* display_egl_ptr =
        gl::GLDisplayEGL::GetDisplayForCurrentContext();
    if (!display_egl_ptr || !display_egl_ptr->ext->b_EGL_KHR_fence_sync) {
      LOG(ERROR) << __func__ << " context does not have EGL_KHR_fence_sync";
      return false;
    }
#endif
  } else {
    LOG(INFO) << __func__
              << " No GL callbacks provided, initializing without GL support";
  }

  SetDecoderState(mcil::kInitialized);

  if (!decoder_thread_.Start()) {
    LOG(ERROR) << " decoder thread failed to start";
    return false;
  }

  bool result = false;
  base::WaitableEvent done;
  decoder_thread_.task_runner()->PostTask(
      FROM_HERE,
      base::BindOnce(&WebOSVideoDecodeAccelerator::InitializeTask,
                     base::Unretained(this), config, &result, &done));
  done.Wait();

  LOG(INFO) << __func__ << " : " << (result ? " SUCCESS" : "FAILED");
  return result;
}

void WebOSVideoDecodeAccelerator::Decode(BitstreamBuffer bitstream_buffer) {
  Decode(bitstream_buffer.ToDecoderBuffer(), bitstream_buffer.id());
}

void WebOSVideoDecodeAccelerator::Decode(scoped_refptr<DecoderBuffer> buffer,
                                         int32_t bitstream_id) {
  VLOG(2) << __func__ << " input_id=" << bitstream_id
          << ", size=" << (buffer ? buffer->size() : 0);
  DCHECK(decode_task_runner_->RunsTasksInCurrentSequence());

  if (bitstream_id < 0) {
    LOG(ERROR) << " Invalid bitstream buffer, id: " << bitstream_id;
    NOTIFY_ERROR(INVALID_ARGUMENT);
    return;
  }

  decoder_thread_.task_runner()->PostTask(
      FROM_HERE,
      base::BindOnce(&WebOSVideoDecodeAccelerator::DecodeTask,
                     base::Unretained(this), std::move(buffer), bitstream_id));
}

void WebOSVideoDecodeAccelerator::AssignPictureBuffers(
    const std::vector<PictureBuffer>& buffers) {
  VLOG(2) << __func__ << " buffer_count=" << buffers.size();
  DCHECK(child_task_runner_->RunsTasksInCurrentSequence());

  decoder_thread_.task_runner()->PostTask(
      FROM_HERE,
      base::BindOnce(&WebOSVideoDecodeAccelerator::AssignPictureBuffersTask,
                     base::Unretained(this), buffers));
}

void WebOSVideoDecodeAccelerator::ImportBufferForPicture(
    int32_t picture_buffer_id,
    VideoPixelFormat pixel_format,
    gfx::GpuMemoryBufferHandle gpu_memory_buffer_handle) {
  VLOG(2) << __func__ << " picture_buffer_id=" << picture_buffer_id;

  DCHECK(child_task_runner_->RunsTasksInCurrentSequence());

  if (output_mode_ != Config::OutputMode::kImport) {
    LOG(ERROR) << __func__ << " Cannot import in non-import mode";
    NOTIFY_ERROR(INVALID_ARGUMENT);
    return;
  }

  decoder_thread_.task_runner()->PostTask(
      FROM_HERE,
      base::BindOnce(
          &WebOSVideoDecodeAccelerator::ImportBufferForPictureTaskInternal,
          base::Unretained(this), picture_buffer_id, pixel_format,
          std::move(gpu_memory_buffer_handle.native_pixmap_handle)));
}

void WebOSVideoDecodeAccelerator::ReusePictureBuffer(int32_t pic_buffer_id) {
  VLOG(2) << __func__ << " pic_buffer_id=" << pic_buffer_id;

  DCHECK(child_task_runner_->RunsTasksInCurrentSequence());

  std::unique_ptr<gl::GLFenceEGL> egl_fence;

  if (make_context_current_cb_) {
    if (!make_context_current_cb_.Run()) {
      LOG(ERROR) << " could not make context current";
      NOTIFY_ERROR(PLATFORM_FAILURE);
      return;
    }

#if defined(ARCH_CPU_ARMEL)
    egl_fence = gl::GLFenceEGL::Create();
    if (!egl_fence) {
      LOG(ERROR) << __func__ << " gl::GLFenceEGL::Create() failed";
      NOTIFY_ERROR(PLATFORM_FAILURE);
      return;
    }
#endif
  }

  decoder_thread_.task_runner()->PostTask(
      FROM_HERE,
      base::BindOnce(&WebOSVideoDecodeAccelerator::ReusePictureBufferTask,
                     base::Unretained(this), pic_buffer_id,
                     std::move(egl_fence)));
}

void WebOSVideoDecodeAccelerator::Flush() {
  VLOG(2) << __func__;
  DCHECK(child_task_runner_->RunsTasksInCurrentSequence());
  decoder_thread_.task_runner()->PostTask(
      FROM_HERE, base::BindOnce(&WebOSVideoDecodeAccelerator::FlushTask,
                                base::Unretained(this)));
}

void WebOSVideoDecodeAccelerator::Reset() {
  VLOG(2) << __func__;
  DCHECK(child_task_runner_->RunsTasksInCurrentSequence());
  decoder_thread_.task_runner()->PostTask(
      FROM_HERE, base::BindOnce(&WebOSVideoDecodeAccelerator::ResetTask,
                                base::Unretained(this)));
}

void WebOSVideoDecodeAccelerator::Destroy() {
  VLOG(2) << __func__;
  DCHECK(child_task_runner_->RunsTasksInCurrentSequence());

  destroy_pending_.Signal();

  client_ptr_factory_.reset();
  weak_this_factory_.InvalidateWeakPtrs();

  if (decoder_thread_.IsRunning()) {
    decoder_thread_.task_runner()->PostTask(
        FROM_HERE, base::BindOnce(&WebOSVideoDecodeAccelerator::DestroyTask,
                                  base::Unretained(this)));
    decoder_thread_.Stop();
  }

  LOG(INFO) << __func__ << " Destroyed.";
  delete this;
}

bool WebOSVideoDecodeAccelerator::TryToSetupDecodeOnSeparateSequence(
    const base::WeakPtr<Client>& decode_client,
    const scoped_refptr<base::SequencedTaskRunner>& decode_task_runner) {
  VLOG(2) << __func__;
  decode_client_ = decode_client;
  decode_task_runner_ = decode_task_runner;
  return true;
}

bool WebOSVideoDecodeAccelerator::CreateOutputBuffers(
    mcil::VideoPixelFormat pixel_format,
    uint32_t buffer_count,
    uint32_t texture_target) {
  DCHECK(decoder_thread_.task_runner()->BelongsToCurrentThread());

  VLOG(2) << __func__ << " buffer_count=" << buffer_count
          << ", coded_size=" << egl_image_size_.ToString();
  VideoPixelFormat format = VideoPixelFormatFrom(pixel_format);
  child_task_runner_->PostTask(
      FROM_HERE, base::BindOnce(&Client::ProvidePictureBuffersWithVisibleRect,
                                client_, buffer_count, format, egl_image_size_,
                                gfx::Rect(visible_size_)));

  SetDecoderState(mcil::kAwaitingPictureBuffers);
  return true;
}

bool WebOSVideoDecodeAccelerator::DestroyOutputBuffers() {
  VLOG(2) << __func__;
  DCHECK(decoder_thread_.task_runner()->BelongsToCurrentThread());

  if (output_buffer_map_.empty()) {
    return false;
  }

  // Release all buffers waiting for an import buffer event
  output_wait_map_.clear();

  for (size_t i = 0; i < output_buffer_map_.size(); ++i) {
    OutputRecord& output_record = output_buffer_map_[i];

    if (output_record.egl_image != EGL_NO_IMAGE_KHR) {
      child_task_runner_->PostTask(
          FROM_HERE,
          base::BindOnce(
              base::IgnoreResult(&vda::WebOSVideoUtils::DestroyEGLImage),
              webos_video_utils_, egl_display_, output_record.egl_image));
    }

    VLOG(2) << __func__
            << " dismissing PictureBuffer id=" << output_record.picture_id;
    child_task_runner_->PostTask(
        FROM_HERE, base::BindOnce(&Client::DismissPictureBuffer, client_,
                                  output_record.picture_id));
  }

  while (!buffers_awaiting_fence_.empty())
    buffers_awaiting_fence_.pop();

  output_buffer_map_.clear();

  return true;
}

void WebOSVideoDecodeAccelerator::ScheduleDecodeBufferTaskIfNeeded() {
  VLOG(2) << __func__;

  DCHECK(decoder_thread_.task_runner()->BelongsToCurrentThread());

  int buffers_to_decode = decoder_input_queue_.size();
  if (decoder_current_bitstream_buffer_ != NULL)
    buffers_to_decode++;
  if (decoder_decode_buffer_tasks_scheduled_ < buffers_to_decode) {
    decoder_decode_buffer_tasks_scheduled_++;
    decoder_thread_.task_runner()->PostTask(
        FROM_HERE,
        base::BindOnce(&WebOSVideoDecodeAccelerator::DecodeBufferTask,
                       base::Unretained(this)));
  }
}

void WebOSVideoDecodeAccelerator::StartResolutionChange() {
  VLOG(2) << __func__;

  DCHECK(decoder_thread_.task_runner()->BelongsToCurrentThread());

  if (should_control_buffer_feed_)
    egl_image_creation_completed_ = false;

  SetDecoderState(mcil::kChangingResolution);
  SendPictureReady();

  buffers_at_client_.clear();
}

void WebOSVideoDecodeAccelerator::NotifyFlushDone() {
  DCHECK(decoder_thread_.task_runner()->BelongsToCurrentThread());

  decoder_delay_bitstream_buffer_id_ = -1;
  decoder_flushing_ = false;
  VLOG(2) << __func__ << " returning flush";
  child_task_runner_->PostTask(
      FROM_HERE, base::BindOnce(&Client::NotifyFlushDone, client_));
}

void WebOSVideoDecodeAccelerator::NotifyFlushDoneIfNeeded() {
  VLOG(2) << __func__;

  DCHECK(decoder_thread_.task_runner()->BelongsToCurrentThread());

  if (!decoder_flushing_)
    return;

  if (!decoder_input_queue_.empty()) {
    if (decoder_input_queue_.front()->input_id !=
        decoder_delay_bitstream_buffer_id_) {
      DVLOG(3) << __func__ << " Some input bitstream buffers are not queued.";
      return;
    }
  }

  if (!video_decoder_api_->DidFlushBuffersDone())
    return;

  NotifyFlushDone();

  ScheduleDecodeBufferTaskIfNeeded();
}

void WebOSVideoDecodeAccelerator::NotifyResetDone() {
  VLOG(2) << __func__;

  NotifyFlushDoneIfNeeded();

  SetDecoderState(mcil::kResetting);

  SendPictureReady();
  decoder_thread_.task_runner()->PostTask(
      FROM_HERE, base::BindOnce(&WebOSVideoDecodeAccelerator::ResetDoneTask,
                                base::Unretained(this)));
}

bool WebOSVideoDecodeAccelerator::IsDestroyPending() {
  VLOG(2) << __func__;
  return destroy_pending_.IsSignaled();
}

void WebOSVideoDecodeAccelerator::OnStartDevicePoll() {
  VLOG(2) << __func__;

  DCHECK(decoder_thread_.task_runner()->BelongsToCurrentThread());

  if (!decode_buffer_task_callback_) {
    decode_buffer_task_.Reset(
        base::BindRepeating(&WebOSVideoDecodeAccelerator::RunDecodeBufferTask,
                            base::Unretained(this)));
    decode_buffer_task_callback_ = decode_buffer_task_.callback();
  }

  if (!decode_post_task_callback_) {
    decode_post_task_.Reset(
        base::BindRepeating(&WebOSVideoDecodeAccelerator::RunDecoderPostTask,
                            base::Unretained(this)));
    decode_post_task_callback_ = decode_post_task_.callback();
  }
}

void WebOSVideoDecodeAccelerator::OnStopDevicePoll() {
  VLOG(2) << __func__;

  decode_buffer_task_.Cancel();
  decode_buffer_task_callback_ = base::NullCallback();

  decode_post_task_.Cancel();
  decode_post_task_callback_ = base::NullCallback();
}

void WebOSVideoDecodeAccelerator::CreateBuffersForFormat(
    const mcil::Size& coded_size,
    const mcil::Size& visible_size) {
  VLOG(2) << __func__;

  base::AutoLock auto_lock(lock_);

  egl_image_size_.set_width(coded_size.width);
  egl_image_size_.set_height(coded_size.height);

  visible_size_.set_width(visible_size.width);
  visible_size_.set_height(visible_size.height);

  coded_size_.set_width(coded_size.width);
  coded_size_.set_height(coded_size.height);
}

void WebOSVideoDecodeAccelerator::SendBufferToClient(
    size_t buffer_index,
    int32_t buffer_id,
    mcil::ReadableBufferRef buffer) {
  VLOG(2) << __func__;

  if (!decoder_thread_.task_runner()->BelongsToCurrentThread()) {
    decoder_thread_.task_runner()->PostTask(
        FROM_HERE,
        base::BindOnce(&WebOSVideoDecodeAccelerator::SendBufferToClient,
                       weak_this_, buffer_index, buffer_id, buffer));
    return;
  }

  OutputRecord& output_record = output_buffer_map_[buffer_index];
  DCHECK_EQ(buffers_at_client_.count(output_record.picture_id), 0u);

  buffers_at_client_.emplace(output_record.picture_id, std::move(buffer));

  const Picture picture(output_record.picture_id, buffer_id,
                        gfx::Rect(visible_size_), gfx::ColorSpace(), false);
  pending_picture_ready_.emplace(output_record.cleared, picture);
  SendPictureReady();

  output_record.cleared = true;
}

void WebOSVideoDecodeAccelerator::CheckGLFences() {
  VLOG(2) << __func__;

  if (!decoder_thread_.task_runner()->BelongsToCurrentThread()) {
    decoder_thread_.task_runner()->PostTask(
        FROM_HERE, base::BindOnce(&WebOSVideoDecodeAccelerator::CheckGLFences,
                                  weak_this_));
    return;
  }

  while (!buffers_awaiting_fence_.empty()) {
    if (buffers_awaiting_fence_.front().first->HasCompleted()) {
      buffers_awaiting_fence_.pop();
    } else {
      if (video_decoder_api_->GetFreeBuffersCount(mcil::OUTPUT_QUEUE) == 0) {
        constexpr int64_t resched_delay = 17;
        decoder_thread_.task_runner()->PostDelayedTask(
            FROM_HERE,
            base::BindOnce(&WebOSVideoDecodeAccelerator::Enqueue,
                           base::Unretained(this)),
            base::Milliseconds(resched_delay));
      }
      break;
    }
  }
}

void WebOSVideoDecodeAccelerator::NotifyDecoderError(
    mcil::DecoderError error_code) {
  LOG(ERROR) << __func__ << " error_code: " << error_code;

  DCHECK(decoder_thread_.task_runner()->BelongsToCurrentThread());
  NOTIFY_ERROR(static_cast<Error>(error_code));
}

void WebOSVideoDecodeAccelerator::NotifyDecodeBufferTask(bool evt_pending,
                                                         bool has_output) {
  VLOG(2) << __func__;

  if (!decoder_thread_.task_runner()) {
    LOG(ERROR) << __func__ << " decoder thread is not running.";
    return;
  }

  if (decode_buffer_task_callback_) {
    decoder_thread_.task_runner()->PostTask(
        FROM_HERE,
        base::BindOnce(decode_buffer_task_callback_, evt_pending, has_output));
  }
}

void WebOSVideoDecodeAccelerator::NotifyDecoderPostTask(mcil::PostTaskType task,
                                                        bool value) {
  VLOG(2) << __func__;

  if (!decoder_thread_.task_runner()) {
    LOG(ERROR) << __func__ << " decoder thread is not running.";
    return;
  }

  if (decode_post_task_callback_) {
    decoder_thread_.task_runner()->PostTask(
        FROM_HERE, base::BindOnce(decode_post_task_callback_,
                                  static_cast<int>(task), value));
  }
}

void WebOSVideoDecodeAccelerator::NotifyDecodeBufferDone() {
  VLOG(2) << __func__;

  if (!decoder_thread_.task_runner()) {
    LOG(ERROR) << __func__ << " decoder thread is not running.";
    return;
  }

  if (!decoder_thread_.task_runner()->BelongsToCurrentThread()) {
    decoder_thread_.task_runner()->PostTask(
        FROM_HERE,
        base::BindOnce(&WebOSVideoDecodeAccelerator::NotifyDecodeBufferDone,
                       weak_this_));
    return;
  }

  if (should_control_buffer_feed_) {
    if (!decoder_input_release_queue_.empty())
      decoder_input_release_queue_.pop_front();
  }
}

void WebOSVideoDecodeAccelerator::NotifyError(Error error) {
  LOG(ERROR) << __func__ << " error: " << error;

  if (!child_task_runner_->RunsTasksInCurrentSequence()) {
    child_task_runner_->PostTask(
        FROM_HERE, base::BindOnce(&WebOSVideoDecodeAccelerator::NotifyError,
                                  weak_this_, error));
    return;
  }

  if (client_) {
    client_->NotifyError(error);
    client_ptr_factory_.reset();
  }
}

void WebOSVideoDecodeAccelerator::SetErrorState(Error error) {
  LOG(ERROR) << __func__ << " Error code:" << error;

  // We can touch decoder_state_ only if this is the decoder thread or the
  // decoder thread isn't running.
  if (decoder_thread_.task_runner() &&
      !decoder_thread_.task_runner()->BelongsToCurrentThread()) {
    decoder_thread_.task_runner()->PostTask(
        FROM_HERE, base::BindOnce(&WebOSVideoDecodeAccelerator::SetErrorState,
                                  weak_this_, error));
    return;
  }

  if (decoder_state_ != mcil::kDecoderError &&
      decoder_state_ != mcil::kUninitialized &&
      decoder_state_ != mcil::kDestroying) {
    NotifyError(error);
  }

  SetDecoderState(mcil::kDecoderError);
}

void WebOSVideoDecodeAccelerator::SetDecoderState(mcil::CodecState state) {
  if (decoder_state_ == state)
    return;

  VLOG(2) << __func__ << " decoder_state_[ " << decoder_state_ << " -> "
          << state << " ]";
  decoder_state_ = state;
  video_decoder_api_->SetDecoderState(decoder_state_);
}

void WebOSVideoDecodeAccelerator::InitializeTask(const Config& config,
                                                 bool* result,
                                                 base::WaitableEvent* done) {
  VLOG(2) << __func__ << " config: " << config.AsHumanReadableString();

  DCHECK(decoder_thread_.task_runner()->BelongsToCurrentThread());
  DCHECK_NE(result, nullptr);
  DCHECK_NE(done, nullptr);
  DCHECK_EQ(decoder_state_, mcil::kInitialized);
  DCHECK(video_decoder_api_);

  VideoCodec video_codec = VideoCodecProfileToVideoCodec(config.profile);
  mcil::DecoderConfig decoder_config;
  decoder_config.frameWidth = config.initial_expected_coded_size.width();
  decoder_config.frameHeight = config.initial_expected_coded_size.height();
  decoder_config.profile = MCILVideoCodecProfileFrom(config.profile);
  decoder_config.outputMode = static_cast<mcil::OutputMode>(config.output_mode);
  mcil::DecoderClientConfig client_config;
  bool init_result =
      video_decoder_api_->Initialize(&decoder_config, &client_config);

  output_pixel_format_ = client_config.output_pixel_format;
  should_control_buffer_feed_ = client_config.should_control_buffer_feed;
  output_mode_ = config.output_mode;
  egl_image_creation_completed_ = !should_control_buffer_feed_;

  *result = init_result;
  done->Signal();

  if (!init_result) {
    LOG(ERROR) << __func__ << " Failed to create decoder instance.";
    return;
  }

  frame_splitter_ = vda::InputBufferFragmentSplitter::CreateFromProfile(
      config.profile, !should_control_buffer_feed_);
  if (!frame_splitter_) {
    NOTIFY_ERROR(INVALID_ARGUMENT);
    return;
  }
}

void WebOSVideoDecodeAccelerator::DecodeTask(
    scoped_refptr<DecoderBuffer> buffer,
    int32_t bitstream_id) {
  VLOG(2) << __func__ << " input_id=" << bitstream_id;
  DCHECK(decoder_thread_.task_runner()->BelongsToCurrentThread());
  DCHECK_NE(decoder_state_, mcil::kUninitialized);

  if (IsDestroyPending())
    return;

  std::unique_ptr<BitstreamBufferRef> bitstream_record(new BitstreamBufferRef(
      decode_client_, decode_task_runner_, std::move(buffer), bitstream_id));

  if (!bitstream_record->buffer)
    return;

  if (decoder_state_ == mcil::kResetting || decoder_flushing_) {
    if (decoder_delay_bitstream_buffer_id_ == -1)
      decoder_delay_bitstream_buffer_id_ = bitstream_record->input_id;
  } else if (decoder_state_ == mcil::kDecoderError) {
    VLOG(2) << __func__ << " early out: kError state";
    return;
  }

  decoder_input_queue_.push_back(std::move(bitstream_record));
  decoder_decode_buffer_tasks_scheduled_++;
  DecodeBufferTask();
}

void WebOSVideoDecodeAccelerator::AssignPictureBuffersTask(
    const std::vector<PictureBuffer>& buffers) {
  VLOG(2) << __func__;

  DCHECK(decoder_thread_.task_runner()->BelongsToCurrentThread());
  DCHECK_EQ(decoder_state_, mcil::kAwaitingPictureBuffers);

  if (IsDestroyPending())
    return;

  output_buffer_map_.resize(buffers.size());

  std::vector<mcil::WritableBufferRef*> writable_buffers;
  writable_buffers.resize(buffers.size());
  if (!video_decoder_api_->AllocateOutputBuffers(buffers.size(),
                                                 writable_buffers)) {
    LOG(ERROR) << __func__ << " Error allocating output buffer!";
    return;
  }

  for (mcil::WritableBufferRef* buffer : writable_buffers) {
    const int i = buffer->BufferIndex();

    OutputRecord& output_record = output_buffer_map_[i];
    DCHECK_EQ(output_record.egl_image, EGL_NO_IMAGE_KHR);
    DCHECK_EQ(output_record.picture_id, -1);
    DCHECK(!output_record.cleared);

    output_record.picture_id = buffers[i].id();

    DCHECK_EQ(output_wait_map_.count(buffers[i].id()), 0u);
    output_wait_map_.emplace(buffers[i].id(),
                             std::unique_ptr<mcil::WritableBufferRef>(buffer));

    if (output_mode_ == Config::OutputMode::kAllocate) {
      gfx::NativePixmapHandle np_handle;
      std::vector<base::ScopedFD> dmabuf_fds;

      mcil::scoped_refptr<mcil::VideoFrame> mcil_video_frame =
          buffer->GetVideoFrame();
      for (size_t i = 0; i < mcil_video_frame->dmabuf_fds.size(); ++i) {
        dmabuf_fds.push_back(base::ScopedFD(mcil_video_frame->dmabuf_fds[i]));
      }

      size_t num_planes = mcil_video_frame->color_planes.size();
      if (dmabuf_fds.size() < num_planes) {
        while (num_planes != dmabuf_fds.size()) {
          dmabuf_fds.emplace_back(dmabuf_fds.back().get());
        }
      }

      for (size_t i = 0; i < num_planes; ++i) {
        np_handle.planes.emplace_back(mcil_video_frame->color_planes[i].stride,
                                      mcil_video_frame->color_planes[i].offset,
                                      mcil_video_frame->color_planes[i].size,
                                      std::move(dmabuf_fds[i]));
      }
      ImportBufferForPictureTask(output_record.picture_id,
                                 std::move(np_handle));
    }
  }

  if (output_mode_ == Config::OutputMode::kAllocate) {
    ScheduleDecodeBufferTaskIfNeeded();
  }
}

void WebOSVideoDecodeAccelerator::ImportBufferForPictureTaskInternal(
    int32_t picture_buffer_id,
    VideoPixelFormat pixel_format,
    gfx::NativePixmapHandle handle) {
  VLOG(2) << __func__ << " picture_buffer_id: " << picture_buffer_id;

  DCHECK(decoder_thread_.task_runner()->BelongsToCurrentThread());

  if (pixel_format != VideoPixelFormatFrom(output_pixel_format_)) {
    LOG(ERROR) << " Unsupported import format: " << pixel_format;
    NOTIFY_ERROR(INVALID_ARGUMENT);
    return;
  }

  ImportBufferForPictureTask(picture_buffer_id, std::move(handle));
}

void WebOSVideoDecodeAccelerator::ImportBufferForPictureTask(
    int32_t picture_buffer_id,
    gfx::NativePixmapHandle handle) {
  VLOG(2) << __func__ << " picture_buffer_id=" << picture_buffer_id
          << ", handle.planes.size()=" << handle.planes.size();
  DCHECK(decoder_thread_.task_runner()->BelongsToCurrentThread());

  if (IsDestroyPending())
    return;

  const auto iter =
      std::find_if(output_buffer_map_.begin(), output_buffer_map_.end(),
                   [picture_buffer_id](const OutputRecord& output_record) {
                     return output_record.picture_id == picture_buffer_id;
                   });
  if (iter == output_buffer_map_.end()) {
    VLOG(2) << __func__ << " got picture id=" << picture_buffer_id
            << " not in use (anymore?).";
    return;
  }

  if (!output_wait_map_.count(iter->picture_id)) {
    LOG(ERROR) << __func__ << " Passed buffer is not waiting to be imported";
    NOTIFY_ERROR(INVALID_ARGUMENT);
    return;
  }

  if (reset_pending_) {
    FinishReset();
  }

  if (decoder_state_ == mcil::kAwaitingPictureBuffers) {
    SetDecoderState(mcil::kDecoding);
    VLOG(2) << __func__ << " Change state to kDecoding";
  }

  if (iter->texture_id != 0) {
    if (iter->egl_image != EGL_NO_IMAGE_KHR) {
      child_task_runner_->PostTask(
          FROM_HERE,
          base::BindOnce(
              base::IgnoreResult(&vda::WebOSVideoUtils::DestroyEGLImage),
              webos_video_utils_, egl_display_, iter->egl_image));
    }

    DCHECK_GT(handle.planes.size(), 0u);
    size_t index = iter - output_buffer_map_.begin();
    child_task_runner_->PostTask(
        FROM_HERE,
        base::BindOnce(&WebOSVideoDecodeAccelerator::CreateEGLImageFor,
                       weak_this_, index, picture_buffer_id, std::move(handle),
                       iter->texture_id, visible_size_, output_pixel_format_));
    return;
  }

  DCHECK_EQ(output_wait_map_.count(picture_buffer_id), 1u);
  output_wait_map_.erase(picture_buffer_id);
  if (decoder_state_ != mcil::kChangingResolution) {
    video_decoder_api_->EnqueueBuffers();
    ScheduleDecodeBufferTaskIfNeeded();
  }
}

void WebOSVideoDecodeAccelerator::ReusePictureBufferTask(
    int32_t picture_buffer_id,
    std::unique_ptr<gl::GLFenceEGL> egl_fence) {
  VLOG(2) << __func__ << " picture_buffer_id=" << picture_buffer_id;

  DCHECK(decoder_thread_.task_runner()->BelongsToCurrentThread());

  if (IsDestroyPending())
    return;

  if (decoder_state_ == mcil::kDecoderError) {
    LOG(ERROR) << __func__ << " early out: kError state";
    return;
  }

  if (decoder_state_ == mcil::kChangingResolution) {
    LOG(ERROR) << __func__ << " early out: kChangingResolution";
    return;
  }

  auto iter = buffers_at_client_.find(picture_buffer_id);
  if (iter == buffers_at_client_.end()) {
    DVLOG(3) << "got picture id= " << picture_buffer_id
             << " not in use (anymore?).";
    return;
  }

  if (egl_fence)
    buffers_awaiting_fence_.emplace(
        std::make_pair(std::move(egl_fence), std::move(iter->second)));

  buffers_at_client_.erase(iter);

  video_decoder_api_->ReusePictureBuffer(picture_buffer_id);
  video_decoder_api_->EnqueueBuffers();
}

void WebOSVideoDecodeAccelerator::FlushTask() {
  VLOG(2) << __func__;

  DCHECK(decoder_thread_.task_runner()->BelongsToCurrentThread());

  if (IsDestroyPending())
    return;

  if (decoder_state_ == mcil::kDecoderError) {
    LOG(ERROR) << __func__ << " early out: kError state";
    return;
  }

  DCHECK(!decoder_flushing_);

  decoder_input_queue_.push_back(std::make_unique<BitstreamBufferRef>(
      decode_client_, decode_task_runner_, nullptr, kFlushBufferId));

  decoder_flushing_ = true;
  SendPictureReady();

  ScheduleDecodeBufferTaskIfNeeded();
}

void WebOSVideoDecodeAccelerator::ResetTask() {
  VLOG(2) << __func__;

  DCHECK(decoder_thread_.task_runner()->BelongsToCurrentThread());

  if (IsDestroyPending())
    return;

  if (decoder_state_ == mcil::kDecoderError) {
    LOG(ERROR) << __func__ << " early out: kError state";
    return;
  }

  decoder_current_bitstream_buffer_.reset();
  while (!decoder_input_queue_.empty())
    decoder_input_queue_.pop_front();

  while (!decoder_input_release_queue_.empty())
    decoder_input_release_queue_.pop_front();

  video_decoder_api_->ResetInputBuffer();

  DCHECK(!reset_pending_);
  if (decoder_state_ == mcil::kChangingResolution ||
      decoder_state_ == mcil::kAwaitingPictureBuffers) {
    reset_pending_ = true;
    return;
  }

  FinishReset();
}

void WebOSVideoDecodeAccelerator::DestroyTask() {
  VLOG(2) << __func__;
  DCHECK(decoder_thread_.task_runner()->BelongsToCurrentThread());

  SetDecoderState(mcil::kDestroying);

  video_decoder_api_->Destroy();

  decoder_current_bitstream_buffer_.reset();

  decoder_decode_buffer_tasks_scheduled_ = 0;
  while (!decoder_input_queue_.empty())
    decoder_input_queue_.pop_front();

  while (!decoder_input_release_queue_.empty())
    decoder_input_release_queue_.pop_front();

  decoder_flushing_ = false;

  buffers_at_client_.clear();

  frame_splitter_ = nullptr;
}

void WebOSVideoDecodeAccelerator::DecodeBufferTask() {
  VLOG(2) << __func__;

  DCHECK(decoder_thread_.task_runner()->BelongsToCurrentThread());
  DCHECK_NE(decoder_state_, mcil::kUninitialized);

  if (IsDestroyPending())
    return;

  decoder_decode_buffer_tasks_scheduled_--;

  if (!egl_image_creation_completed_) {
    VLOG(2) << __func__ << " egl images are not created";
    return;
  }

  if (decoder_state_ != mcil::kInitialized &&
      decoder_state_ != mcil::kDecoding) {
    DVLOG(3) << __func__ << " early out: state=" << decoder_state_;
    return;
  }

  if (decoder_current_bitstream_buffer_ == NULL) {
    if (decoder_input_queue_.empty()) {
      return;
    }
    if (decoder_delay_bitstream_buffer_id_ ==
        decoder_input_queue_.front()->input_id) {
      return;
    }

    decoder_current_bitstream_buffer_ = std::move(decoder_input_queue_.front());
    decoder_input_queue_.pop_front();
    const auto& buffer = decoder_current_bitstream_buffer_->buffer;
    if (buffer) {
      DVLOG(4) << __func__ << " reading input_id="
               << decoder_current_bitstream_buffer_->input_id
               << ", addr=" << buffer->data() << ", size=" << buffer->size();
    } else {
      DCHECK_EQ(decoder_current_bitstream_buffer_->input_id, kFlushBufferId);
      DVLOG(4) << __func__ << " reading input_id=kFlushBufferId";
    }
  }

  bool schedule_task = false;
  size_t decoded_size = 0;
  const auto& buffer = decoder_current_bitstream_buffer_->buffer;
  if (!buffer) {
    DCHECK_EQ(decoder_current_bitstream_buffer_->input_id, kFlushBufferId);

    schedule_task = true;
    int32_t buffer_id = -1;
    if (video_decoder_api_->GetCurrentInputBufferId(&buffer_id) &&
        (buffer_id != kFlushBufferId)) {
      schedule_task = FlushInputFrame();
    }

    if (schedule_task && AppendToInputFrame(NULL, 0, kFlushBufferId, 0) &&
        FlushInputFrame()) {
      VLOG(2) << __func__ << " enqueued flush buffer";
      schedule_task = true;
    } else {
      schedule_task = false;
    }
  } else if (buffer->empty()) {
    schedule_task = true;
  } else {
    const uint8_t* const data =
        buffer->data() + decoder_current_bitstream_buffer_->bytes_used;
    const size_t data_size =
        buffer->size() - decoder_current_bitstream_buffer_->bytes_used;
    int64_t pts = buffer->timestamp().InNanoseconds();
    int32_t id = decoder_current_bitstream_buffer_->input_id;
    if (!frame_splitter_->AdvanceFrameFragment(data, data_size,
                                               &decoded_size)) {
      LOG(ERROR) << __func__ << " Invalid Stream";
      NOTIFY_ERROR(UNREADABLE_INPUT);
      return;
    }

    CHECK_LE(decoded_size, data_size);

    switch (decoder_state_) {
      case mcil::kInitialized:
        schedule_task =
            DecodeBufferInitial(data, decoded_size, id, pts, &decoded_size);
        break;
      case mcil::kDecoding:
        schedule_task = DecodeBufferContinue(data, decoded_size, id, pts);
        break;
      default:
        LOG(ERROR) << __func__ << " Illegal State";
        NOTIFY_ERROR(ILLEGAL_STATE);
        return;
    }
  }

  if (decoder_state_ == mcil::kDecoderError) {
    return;
  }

  if (schedule_task) {
    decoder_current_bitstream_buffer_->bytes_used += decoded_size;
    if ((buffer ? buffer->size() : 0) ==
        decoder_current_bitstream_buffer_->bytes_used) {
      int32_t input_id = decoder_current_bitstream_buffer_->input_id;
      DVLOG(4) << __func__ << " finished input_id=" << input_id;
      if (should_control_buffer_feed_) {
        decoder_input_release_queue_.push_back(
            std::move(decoder_current_bitstream_buffer_));
      }
      decoder_current_bitstream_buffer_.reset();
    }
    ScheduleDecodeBufferTaskIfNeeded();
  }
}

void WebOSVideoDecodeAccelerator::RunDecodeBufferTask(bool event_pending,
                                                      bool has_output) {
  VLOG(2) << __func__ << " event_pending: " << event_pending;

  if (IsDestroyPending())
    return;

  if (decoder_state_ == mcil::kResetting) {
    VLOG(2) << __func__ << "early out: kResetting state";
    return;
  } else if (decoder_state_ == mcil::kDecoderError) {
    VLOG(2) << __func__ << "early out: kError state";
    return;
  } else if (decoder_state_ == mcil::kChangingResolution) {
    VLOG(2) << __func__ << "early out: kChangingResolution state";
    return;
  }

  video_decoder_api_->RunDecodeBufferTask(event_pending, has_output);
}

void WebOSVideoDecodeAccelerator::RunDecoderPostTask(int task, bool value) {
  VLOG(2) << __func__ << " task: " << task << ", value: " << value;

  if (IsDestroyPending())
    return;

  if (decoder_state_ == mcil::kResetting) {
    VLOG(2) << __func__ << "early out: kResetting state";
    return;
  } else if (decoder_state_ == mcil::kDecoderError) {
    VLOG(2) << __func__ << "early out: kError state";
    return;
  }

  video_decoder_api_->RunDecoderPostTask(static_cast<mcil::PostTaskType>(task),
                                         value);
}

bool WebOSVideoDecodeAccelerator::DecodeBufferInitial(const void* data,
                                                      size_t size,
                                                      int32_t id,
                                                      int64_t pts,
                                                      size_t* endpos) {
  VLOG(2) << __func__ << " data=" << data << ", size=" << size;

  DCHECK(decoder_thread_.task_runner()->BelongsToCurrentThread());
  DCHECK_EQ(decoder_state_, mcil::kInitialized);

  if (!AppendToInputFrame(data, size, id, pts))
    return false;

  if (frame_splitter_->IsPartialFramePending())
    return true;

  if (!FlushInputFrame())
    return false;

  video_decoder_api_->DequeueBuffers();

  *endpos = size;

  if (coded_size_.IsEmpty() || output_buffer_map_.empty()) {
    return true;
  }

  SetDecoderState(mcil::kDecoding);

  ScheduleDecodeBufferTaskIfNeeded();
  return true;
}

bool WebOSVideoDecodeAccelerator::DecodeBufferContinue(const void* data,
                                                       size_t size,
                                                       int32_t id,
                                                       int64_t pts) {
  VLOG(2) << __func__ << " data=" << data << ", size=" << size;

  DCHECK(decoder_thread_.task_runner()->BelongsToCurrentThread());
  DCHECK_EQ(decoder_state_, mcil::kDecoding);

  return (AppendToInputFrame(data, size, id, pts) &&
          (frame_splitter_->IsPartialFramePending() || FlushInputFrame()));
}

bool WebOSVideoDecodeAccelerator::AppendToInputFrame(const void* data,
                                                     size_t size,
                                                     int32_t id,
                                                     int64_t pts) {
  VLOG(2) << __func__ << " data=" << data << ", size=" << size << " id: " << id
          << ", pts=" << pts;

  DCHECK(decoder_thread_.task_runner()->BelongsToCurrentThread());
  DCHECK_NE(decoder_state_, mcil::kUninitialized);
  DCHECK_NE(decoder_state_, mcil::kResetting);
  DCHECK_NE(decoder_state_, mcil::kDecoderError);

  if (video_decoder_api_->DecodeBuffer(data, size, id, pts))
    return true;

  return false;
}

bool WebOSVideoDecodeAccelerator::FlushInputFrame() {
  VLOG(2) << __func__;

  DCHECK(decoder_thread_.task_runner()->BelongsToCurrentThread());
  DCHECK_NE(decoder_state_, mcil::kUninitialized);
  DCHECK_NE(decoder_state_, mcil::kResetting);
  DCHECK_NE(decoder_state_, mcil::kDecoderError);

  return video_decoder_api_->FlushInputBuffers();
}

void WebOSVideoDecodeAccelerator::SendPictureReady() {
  VLOG(2) << __func__;

  DCHECK(decoder_thread_.task_runner()->BelongsToCurrentThread());
  bool send_now = (decoder_state_ == mcil::kChangingResolution ||
                   decoder_state_ == mcil::kResetting || decoder_flushing_);
  while (pending_picture_ready_.size() > 0) {
    bool cleared = pending_picture_ready_.front().cleared;
    const Picture& picture = pending_picture_ready_.front().picture;
    if (cleared && picture_clearing_count_ == 0) {
      VLOG(2) << __func__
              << " picture_buffer_id:" << picture.picture_buffer_id()
              << ", input_buffer_id: " << picture.bitstream_buffer_id();
      decode_task_runner_->PostTask(
          FROM_HERE,
          base::BindOnce(&Client::PictureReady, decode_client_, picture));
      pending_picture_ready_.pop();
    } else if (!cleared || send_now) {
      VLOG(2) << __func__
              << " picture_buffer_id:" << picture.picture_buffer_id()
              << ", input_buffer_id: " << picture.bitstream_buffer_id()
              << ", cleared:" << pending_picture_ready_.front().cleared
              << ", decoder_state_:" << decoder_state_
              << ", decoder_flushing_:" << decoder_flushing_
              << ", picture_clearing_count_:" << picture_clearing_count_;

      child_task_runner_->PostTaskAndReply(
          FROM_HERE, base::BindOnce(&Client::PictureReady, client_, picture),
          base::BindOnce(&WebOSVideoDecodeAccelerator::OnPictureCleared,
                         base::Unretained(this)));
      picture_clearing_count_++;
      pending_picture_ready_.pop();
    } else {
      break;
    }
  }
}

void WebOSVideoDecodeAccelerator::OnPictureCleared() {
  VLOG(2) << __func__;

  VLOG(4) << " clearing count=" << picture_clearing_count_;
  DCHECK(decoder_thread_.task_runner()->BelongsToCurrentThread());
  DCHECK_GT(picture_clearing_count_, 0);

  picture_clearing_count_--;
  SendPictureReady();
}

void WebOSVideoDecodeAccelerator::FinishReset() {
  VLOG(2) << __func__;

  DCHECK(decoder_thread_.task_runner()->BelongsToCurrentThread());

  reset_pending_ = false;
  if (!video_decoder_api_->ResetDecodingBuffers(&reset_pending_))
    return;

  NotifyFlushDoneIfNeeded();

  SetDecoderState(mcil::kResetting);

  SendPictureReady();
  decoder_thread_.task_runner()->PostTask(
      FROM_HERE, base::BindOnce(&WebOSVideoDecodeAccelerator::ResetDoneTask,
                                base::Unretained(this)));
}

void WebOSVideoDecodeAccelerator::ResetDoneTask() {
  VLOG(2) << __func__;

  DCHECK(decoder_thread_.task_runner()->BelongsToCurrentThread());

  if (IsDestroyPending())
    return;

  if (decoder_state_ == mcil::kDecoderError) {
    VLOG(2) << __func__ << " early out: kError state";
    return;
  }

  if (!video_decoder_api_->CanNotifyResetDone())
    return;

  frame_splitter_->Reset();

  DCHECK_EQ(decoder_state_, mcil::kResetting);
  SetDecoderState(mcil::kInitialized);

  decoder_delay_bitstream_buffer_id_ = -1;
  child_task_runner_->PostTask(
      FROM_HERE, base::BindOnce(&Client::NotifyResetDone, client_));

  ScheduleDecodeBufferTaskIfNeeded();
}

void WebOSVideoDecodeAccelerator::Enqueue() {
  VLOG(2) << __func__;

  DCHECK(decoder_thread_.task_runner()->BelongsToCurrentThread());
  video_decoder_api_->EnqueueBuffers();
}

void WebOSVideoDecodeAccelerator::CreateEGLImageFor(
    size_t buffer_index,
    int32_t picture_buffer_id,
    gfx::NativePixmapHandle handle,
    GLuint texture_id,
    const gfx::Size& visible_size,
    mcil::VideoPixelFormat pixel_format) {
  VLOG(2) << __func__ << " index=" << buffer_index;

  DCHECK(child_task_runner_->RunsTasksInCurrentSequence());
  DCHECK_NE(texture_id, 0u);

  if (!get_gl_context_cb_ || !make_context_current_cb_) {
    LOG(ERROR) << " GL callbacks required for binding to EGLImages";
    NOTIFY_ERROR(INVALID_ARGUMENT);
    return;
  }

  gl::GLContext* gl_context = get_gl_context_cb_.Run();
  if (!gl_context || !make_context_current_cb_.Run()) {
    LOG(ERROR) << " No GL context";
    NOTIFY_ERROR(PLATFORM_FAILURE);
    return;
  }

  if (!video_decoder_api_->CanCreateEGLImageFrom(pixel_format)) {
    LOG(ERROR) << __func__ << " Unsupported V4L2 pixel format";
    NOTIFY_ERROR(PLATFORM_FAILURE);
    return;
  }

  gl::ScopedTextureBinder bind_restore(GL_TEXTURE_EXTERNAL_OES, 0);
  EGLImageKHR egl_image = webos_video_utils_->CreateEGLImage(
      egl_display_, gl_context->GetHandle(), texture_id, visible_size,
      buffer_index, pixel_format, std::move(handle));
  if (egl_image == EGL_NO_IMAGE_KHR) {
    LOG(ERROR) << " could not create EGLImageKHR,"
               << " index=" << buffer_index << " texture_id=" << texture_id;
    NOTIFY_ERROR(PLATFORM_FAILURE);
    return;
  }

  decoder_thread_.task_runner()->PostTask(
      FROM_HERE, base::BindOnce(&WebOSVideoDecodeAccelerator::AssignEGLImage,
                                base::Unretained(this), buffer_index,
                                picture_buffer_id, egl_image));
}

void WebOSVideoDecodeAccelerator::AssignEGLImage(size_t buffer_index,
                                                 int32_t picture_buffer_id,
                                                 EGLImageKHR egl_image) {
  VLOG(2) << __func__ << " index=" << buffer_index
          << ", picture_id=" << picture_buffer_id;
  DCHECK(decoder_thread_.task_runner()->BelongsToCurrentThread());

  if (IsDestroyPending())
    return;

  if (buffer_index >= output_buffer_map_.size() ||
      output_buffer_map_[buffer_index].picture_id != picture_buffer_id) {
    VLOG(2) << __func__ << " Picture set already changed, dropping EGLImage";
    child_task_runner_->PostTask(
        FROM_HERE, base::BindOnce(base::IgnoreResult(
                                      &vda::WebOSVideoUtils::DestroyEGLImage),
                                  webos_video_utils_, egl_display_, egl_image));
    return;
  }

  OutputRecord& output_record = output_buffer_map_[buffer_index];
  DCHECK_EQ(output_record.egl_image, EGL_NO_IMAGE_KHR);

  output_record.egl_image = egl_image;

  DCHECK_EQ(output_wait_map_.count(picture_buffer_id), 1u);
  output_wait_map_.erase(picture_buffer_id);

  if (!egl_image_creation_completed_) {
    egl_image_creation_completed_ = true;
    LOG(INFO) << __func__ << " first EGL assigned, index=" << buffer_index
              << ", picture_id=" << picture_buffer_id;
    video_decoder_api_->OnEGLImagesCreationCompleted();
  }

  if (decoder_state_ != mcil::kChangingResolution) {
    video_decoder_api_->EnqueueBuffers();
    ScheduleDecodeBufferTaskIfNeeded();
  }
}

}  // namespace media
