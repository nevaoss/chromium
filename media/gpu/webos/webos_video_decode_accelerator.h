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

#ifndef MEDIA_GPU_WEBOS_WEBOS_VIDEO_DECODE_ACCELERATOR_H_
#define MEDIA_GPU_WEBOS_WEBOS_VIDEO_DECODE_ACCELERATOR_H_

#include <stddef.h>
#include <stdint.h>

#include <mcil/video_decoder_client.h>

#include <list>
#include <map>
#include <memory>
#include <queue>
#include <utility>
#include <vector>

#include "base/cancelable_callback.h"
#include "base/containers/queue.h"
#include "base/functional/callback_forward.h"
#include "base/memory/ref_counted.h"
#include "base/synchronization/waitable_event.h"
#include "base/threading/thread.h"
#include "media/base/limits.h"
#include "media/base/video_decoder_config.h"
#include "media/gpu/chromeos/fourcc.h"
#include "media/gpu/chromeos/image_processor.h"
#include "media/gpu/media_gpu_export.h"
#include "media/gpu/v4l2/v4l2_device.h"
#include "media/video/picture.h"
#include "media/video/video_decode_accelerator.h"
#include "ui/gfx/geometry/size.h"
#include "ui/gl/gl_bindings.h"
#include "ui/gl/gl_fence_egl.h"

namespace mcil {
class VideoDecoderAPI;
}  // namespace mcil

namespace gl {
class GLContext;
class GLFenceEGL;
}  // namespace gl

namespace media {

namespace vda {
class InputBufferFragmentSplitter;
class WebOSVideoUtils;
}  // namespace vda

// Return current GLContext.
using GetGLContextCallback = base::RepeatingCallback<gl::GLContext*(void)>;

// Make the applicable GL context current. To be called by VDAs before
// executing any GL calls. Return true on success, false otherwise.
using MakeGLContextCurrentCallback = base::RepeatingCallback<bool(void)>;

class MEDIA_GPU_EXPORT WebOSVideoDecodeAccelerator
    : public VideoDecodeAccelerator,
      public mcil::VideoDecoderClient {
 public:
  static VideoDecodeAccelerator::SupportedProfiles GetSupportedProfiles();

  WebOSVideoDecodeAccelerator(
      EGLDisplay egl_display,
      const GetGLContextCallback& get_gl_context_cb,
      const MakeGLContextCurrentCallback& make_context_current_cb);
  ~WebOSVideoDecodeAccelerator() override;

  // VideoDecodeAccelerator implementation.
  bool Initialize(const Config& config, Client* client) override;
  void Decode(BitstreamBuffer bitstream_buffer) override;
  void Decode(scoped_refptr<DecoderBuffer> buffer,
              int32_t bitstream_id) override;
  void AssignPictureBuffers(const std::vector<PictureBuffer>& buffers) override;
  void ImportBufferForPicture(
      int32_t picture_buffer_id,
      VideoPixelFormat pixel_format,
      gfx::GpuMemoryBufferHandle gpu_memory_buffer_handles) override;
  void ReusePictureBuffer(int32_t picture_buffer_id) override;
  void Flush() override;
  void Reset() override;
  void Destroy() override;
  bool TryToSetupDecodeOnSeparateSequence(
      const base::WeakPtr<Client>& decode_client,
      const scoped_refptr<base::SequencedTaskRunner>& decode_task_runner)
      override;

  // mcil::VideoDecoderClient implementation.
  bool CreateOutputBuffers(mcil::VideoPixelFormat pixel_format,
                           uint32_t buffer_count,
                           uint32_t texture_target) override;
  bool DestroyOutputBuffers() override;
  void ScheduleDecodeBufferTaskIfNeeded() override;
  void StartResolutionChange() override;
  void NotifyFlushDone() override;
  void NotifyFlushDoneIfNeeded() override;
  void NotifyResetDone() override;
  bool IsDestroyPending() override;
  void OnStartDevicePoll() override;
  void OnStopDevicePoll() override;
  void CreateBuffersForFormat(const mcil::Size& coded_size,
                              const mcil::Size& visible_size) override;
  void SendBufferToClient(size_t buffer_index,
                          int32_t buffer_id,
                          mcil::ReadableBufferRef buffer) override;
  void CheckGLFences() override;

  void NotifyDecoderError(mcil::DecoderError error) override;
  void NotifyDecodeBufferTask(bool event_pending, bool has_output) override;
  void NotifyDecoderPostTask(mcil::PostTaskType task, bool value) override;
  void NotifyDecodeBufferDone() override;

 private:
  WebOSVideoDecodeAccelerator(const WebOSVideoDecodeAccelerator&) = delete;
  void operator=(WebOSVideoDecodeAccelerator const&) = delete;

  // Auto-destruction reference for BitstreamBuffer
  struct BitstreamBufferRef;

  // Record for output buffers.
  struct OutputRecord;

  // Record for decoded pictures that can be sent to PictureReady.
  struct PictureRecord {
    PictureRecord(bool cleared, const Picture& picture)
        : cleared(cleared), picture(picture) {}
    ~PictureRecord() = default;
    bool cleared;
    Picture picture;
  };

  void NotifyError(Error error);
  void SetErrorState(Error error);

  void SetDecoderState(mcil::CodecState state);

  void InitializeTask(const Config& config,
                      bool* result,
                      base::WaitableEvent* done);

  void DecodeTask(scoped_refptr<DecoderBuffer> buffer, int32_t bitstream_id);
  void AssignPictureBuffersTask(const std::vector<PictureBuffer>& buffers);
  void ImportBufferForPictureTask(int32_t picture_buffer_id,
                                  gfx::NativePixmapHandle handle);
  void ReusePictureBufferTask(int32_t picture_buffer_id,
                              std::unique_ptr<gl::GLFenceEGL> egl_fence);

  void ImportBufferForPictureTaskInternal(int32_t picture_buffer_id,
                                          VideoPixelFormat pixel_format,
                                          gfx::NativePixmapHandle handle);

  void FlushTask();
  void ResetTask();
  void DestroyTask();

  void DecodeBufferTask();

  bool DecodeBufferInitial(const void* data,
                           size_t size,
                           int32_t id,
                           int64_t pts,
                           size_t* endpos);
  bool DecodeBufferContinue(const void* data,
                            size_t size,
                            int32_t id,
                            int64_t pts);
  bool AppendToInputFrame(const void* data,
                          size_t size,
                          int32_t id,
                          int64_t pts);
  bool FlushInputFrame();

  void RunDecodeBufferTask(bool event_pending, bool has_output);
  void RunDecoderPostTask(int task, bool value);

  void SendPictureReady();
  void OnPictureCleared();
  void FinishReset();
  void ResetDoneTask();

  void Enqueue();

  void CreateEGLImageFor(size_t buffer_index,
                         int32_t picture_buffer_id,
                         gfx::NativePixmapHandle handle,
                         GLuint texture_id,
                         const gfx::Size& visible_size,
                         mcil::VideoPixelFormat pixel_format);
  void AssignEGLImage(size_t buffer_index,
                      int32_t picture_buffer_id,
                      EGLImageKHR egl_image);

  // EGL state
  EGLDisplay egl_display_;

  // Callback to get current GLContext.
  GetGLContextCallback get_gl_context_cb_;

  // Callback to set the correct gl context.
  MakeGLContextCurrentCallback make_context_current_cb_;

  base::Thread decoder_thread_;
  mcil::CodecState decoder_state_;

  scoped_refptr<base::SequencedTaskRunner> child_task_runner_;
  scoped_refptr<base::SequencedTaskRunner> decode_task_runner_;

  base::WaitableEvent destroy_pending_;

  Config::OutputMode output_mode_;

  mcil::VideoPixelFormat output_pixel_format_ = mcil::PIXEL_FORMAT_UNKNOWN;

  int decoder_decode_buffer_tasks_scheduled_ = 0;
  int32_t decoder_delay_bitstream_buffer_id_ = -1;

  std::unique_ptr<BitstreamBufferRef> decoder_current_bitstream_buffer_;

  bool reset_pending_ = false;

  gfx::Size visible_size_;
  gfx::Size egl_image_size_;
  gfx::Size coded_size_;

  std::unique_ptr<vda::InputBufferFragmentSplitter> frame_splitter_;

  base::circular_deque<std::unique_ptr<BitstreamBufferRef>>
      decoder_input_queue_;
  base::circular_deque<std::unique_ptr<BitstreamBufferRef>>
      decoder_input_release_queue_;

  std::map<int32_t, mcil::ReadableBufferRef> buffers_at_client_;
  std::queue<
      std::pair<std::unique_ptr<gl::GLFenceEGL>, mcil::ReadableBufferRef>>
      buffers_awaiting_fence_;

  std::vector<OutputRecord> output_buffer_map_;
  std::map<int32_t, std::unique_ptr<mcil::WritableBufferRef>> output_wait_map_;

  base::queue<PictureRecord> pending_picture_ready_;
  int picture_clearing_count_ = 0;

  std::unique_ptr<base::WeakPtrFactory<Client>> client_ptr_factory_;
  base::WeakPtr<Client> client_;
  base::WeakPtr<Client> decode_client_;

  // The underlying perform decoding instance.
  std::unique_ptr<mcil::VideoDecoderAPI> video_decoder_api_;

  scoped_refptr<vda::WebOSVideoUtils> webos_video_utils_;

  base::CancelableRepeatingCallback<void(bool, bool)> decode_buffer_task_;
  base::RepeatingCallback<void(bool, bool)> decode_buffer_task_callback_;

  base::CancelableRepeatingCallback<void(int, bool)> decode_post_task_;
  base::RepeatingCallback<void(int, bool)> decode_post_task_callback_;

  bool should_control_buffer_feed_ = false;
  bool decoder_flushing_ = false;
  bool egl_image_creation_completed_ = true;

  mutable base::Lock lock_;

  // The WeakPtr for |weak_this_|.
  base::WeakPtr<WebOSVideoDecodeAccelerator> weak_this_;
  // The WeakPtrFactory for |weak_this_|.
  base::WeakPtrFactory<WebOSVideoDecodeAccelerator> weak_this_factory_;
};

}  // namespace media

#endif  // MEDIA_GPU_WEBOS_WEBOS_VIDEO_DECODE_ACCELERATOR_H_
