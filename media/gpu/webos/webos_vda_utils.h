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

#ifndef MEDIA_GPU_WEBOS_WEBOS_VDA_UTILS_H_
#define MEDIA_GPU_WEBOS_WEBOS_VDA_UTILS_H_

#include <mcil/decoder_types.h>

#include "base/memory/scoped_refptr.h"
#include "media/base/video_codecs.h"
#include "media/base/video_frame.h"
#include "media/base/video_types.h"
#include "media/gpu/chromeos/fourcc.h"
#include "media/parsers/h264_parser.h"
#include "ui/gfx/native_pixmap_handle.h"
#include "ui/gl/gl_bindings.h"
#include "ui/gl/gl_fence_egl.h"

namespace media {

enum VideoBufferId {
  kFlushBufferId = -2,
};

namespace vda {

class InputBufferFragmentSplitter {
 public:
  static std::unique_ptr<InputBufferFragmentSplitter> CreateFromProfile(
      media::VideoCodecProfile profile,
      bool use_h264_fragment_splitter);

  explicit InputBufferFragmentSplitter() = default;
  virtual ~InputBufferFragmentSplitter() = default;

  virtual bool AdvanceFrameFragment(const uint8_t* data,
                                    size_t size,
                                    size_t* endpos);
  virtual void Reset() {}
  virtual bool IsPartialFramePending() const;
};

class H264InputBufferFragmentSplitter : public InputBufferFragmentSplitter {
 public:
  explicit H264InputBufferFragmentSplitter();
  ~H264InputBufferFragmentSplitter() override = default;

  bool AdvanceFrameFragment(const uint8_t* data,
                            size_t size,
                            size_t* endpos) override;
  void Reset() override;
  bool IsPartialFramePending() const override;

 private:
  std::unique_ptr<H264Parser> h264_parser_;
  bool partial_frame_pending_ = false;
};

class WebOSVideoUtils : public base::RefCountedThreadSafe<WebOSVideoUtils> {
 public:
  WebOSVideoUtils() = default;
  ~WebOSVideoUtils() = default;

  scoped_refptr<VideoFrame> CreateVideoFrame(
      mcil::scoped_refptr<mcil::VideoFrame> frame);

  EGLImageKHR CreateEGLImage(EGLDisplay egl_display,
                             EGLContext egl_context,
                             GLuint texture_id,
                             const gfx::Size& visible_size,
                             unsigned int buffer_index,
                             mcil::VideoPixelFormat pixel_format,
                             gfx::NativePixmapHandle handle) const;
  EGLBoolean DestroyEGLImage(EGLDisplay egl_display, EGLImageKHR egl_image);

 private:
  std::unique_ptr<H264Parser> h264_parser_;
  bool partial_frame_pending_ = false;
};

}  // namespace vda

}  // namespace media

#endif  // MEDIA_GPU_WEBOS_WEBOS_VDA_UTILS_H_
