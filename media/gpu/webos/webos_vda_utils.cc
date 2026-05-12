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

#include "media/gpu/webos/webos_vda_utils.h"

#include "base/logging.h"
#include "media/base/video_decoder_config.h"
#include "media/gpu/webos/webos_codec_utils.h"
#include "ui/gl/egl_util.h"
#include "ui/ozone/public/ozone_platform.h"
#include "ui/ozone/public/surface_factory_ozone.h"

namespace media {

namespace vda {

// static
std::unique_ptr<InputBufferFragmentSplitter>
InputBufferFragmentSplitter::CreateFromProfile(
    media::VideoCodecProfile profile,
    bool use_h264_fragment_splitter) {
  switch (VideoCodecProfileToVideoCodec(profile)) {
    case VideoCodec::kH264:
      if (use_h264_fragment_splitter)
        return std::make_unique<H264InputBufferFragmentSplitter>();
    case VideoCodec::kVP8:
    case VideoCodec::kVP9:
      // VP8/VP9 don't need any frame splitting, use the default implementation.
      return std::make_unique<InputBufferFragmentSplitter>();
    default:
      LOG(ERROR) << __func__ << " Unhandled profile: " << profile;
      return nullptr;
  }
}

bool InputBufferFragmentSplitter::AdvanceFrameFragment(const uint8_t* data,
                                                       size_t size,
                                                       size_t* endpos) {
  *endpos = size;
  return true;
}

bool InputBufferFragmentSplitter::IsPartialFramePending() const {
  return false;
}

H264InputBufferFragmentSplitter::H264InputBufferFragmentSplitter()
    : h264_parser_(new H264Parser()) {}

bool H264InputBufferFragmentSplitter::AdvanceFrameFragment(const uint8_t* data,
                                                           size_t size,
                                                           size_t* endpos) {
  DCHECK(h264_parser_);

  h264_parser_->SetStream(data, size);
  H264NALU nalu;
  H264Parser::Result result;
  bool has_frame_data = false;
  *endpos = 0;

  while (true) {
    bool end_of_frame = false;
    result = h264_parser_->AdvanceToNextNALU(&nalu);
    if (result == H264Parser::kInvalidStream ||
        result == H264Parser::kUnsupportedStream) {
      return false;
    }
    if (result == H264Parser::kEOStream) {
      if (has_frame_data)
        partial_frame_pending_ = true;
      *endpos = size;
      return true;
    }
    switch (nalu.nal_unit_type) {
      case H264NALU::kNonIDRSlice:
      case H264NALU::kIDRSlice:
        if (nalu.size < 1)
          return false;

        has_frame_data = true;

        if (nalu.data[1] >= 0x80) {
          end_of_frame = true;
          break;
        }
        break;
      case H264NALU::kSEIMessage:
      case H264NALU::kSPS:
      case H264NALU::kPPS:
      case H264NALU::kAUD:
      case H264NALU::kEOSeq:
      case H264NALU::kEOStream:
      case H264NALU::kReserved17:
      case H264NALU::kReserved18:
        end_of_frame = true;
        break;
      default:
        break;
    }
    if (end_of_frame) {
      if (!partial_frame_pending_ && *endpos == 0) {
      } else {
        partial_frame_pending_ = false;
        return true;
      }
    }
    *endpos = (nalu.data + nalu.size) - data;
  }
  return false;
}

void H264InputBufferFragmentSplitter::Reset() {
  partial_frame_pending_ = false;
  h264_parser_.reset(new H264Parser());
}

bool H264InputBufferFragmentSplitter::IsPartialFramePending() const {
  return partial_frame_pending_;
}

scoped_refptr<VideoFrame> WebOSVideoUtils::CreateVideoFrame(
    mcil::scoped_refptr<mcil::VideoFrame> video_frame) {
  VLOG(2) << __func__;

  auto layout = VideoFrameLayoutFrom(video_frame);
  if (!layout) {
    LOG(ERROR) << __func__ << " Cannot create frame layout for format: "
               << video_frame->format;
    return nullptr;
  }

  std::vector<base::ScopedFD> dmabuf_fds;
  for (size_t i = 0; i < video_frame->dmabuf_fds.size(); ++i) {
    dmabuf_fds.push_back(base::ScopedFD(video_frame->dmabuf_fds[i]));
  }

  gfx::Size size(video_frame->coded_size.width, video_frame->coded_size.height);
  return VideoFrame::WrapExternalDmabufs(
      *layout, gfx::Rect(size), size, std::move(dmabuf_fds), base::TimeDelta());
}

EGLImageKHR WebOSVideoUtils::CreateEGLImage(
    EGLDisplay egl_display,
    EGLContext /* egl_context */,
    GLuint texture_id,
    const gfx::Size& size,
    unsigned int buffer_index,
    mcil::VideoPixelFormat pixel_format,
    gfx::NativePixmapHandle handle) const {
  VLOG(2) << __func__ << " texture_id: " << texture_id;

  // Number of components, as opposed to the number of V4L2 planes, which is
  // just a buffer count.
  const size_t num_planes = handle.planes.size();
  DCHECK_LE(num_planes, 3u);

  std::vector<EGLint> attrs;
  attrs.push_back(EGL_WIDTH);
  attrs.push_back(size.width());
  attrs.push_back(EGL_HEIGHT);
  attrs.push_back(size.height());
  attrs.push_back(EGL_LINUX_DRM_FOURCC_EXT);
  attrs.push_back(DrmPixelFormatFrom(pixel_format));

  for (size_t plane = 0; plane < num_planes; ++plane) {
    attrs.push_back(EGL_DMA_BUF_PLANE0_FD_EXT + plane * 3);
    attrs.push_back(handle.planes[plane].fd.get());
    attrs.push_back(EGL_DMA_BUF_PLANE0_OFFSET_EXT + plane * 3);
    attrs.push_back(handle.planes[plane].offset);
    attrs.push_back(EGL_DMA_BUF_PLANE0_PITCH_EXT + plane * 3);
    attrs.push_back(handle.planes[plane].stride);
  }

  attrs.push_back(EGL_NONE);

  EGLImageKHR egl_image = eglCreateImageKHR(
      egl_display, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, NULL, &attrs[0]);
  if (egl_image == EGL_NO_IMAGE_KHR) {
    LOG(ERROR) << __func__
               << " Failed creating EGL image: " << ui::GetLastEGLErrorString();
    return egl_image;
  }

  glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture_id);
  glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, egl_image);

  return egl_image;
}

EGLBoolean WebOSVideoUtils::DestroyEGLImage(EGLDisplay egl_display,
                                            EGLImageKHR egl_image) {
  VLOG(2) << __func__;

  EGLBoolean result = eglDestroyImageKHR(egl_display, egl_image);
  if (result != EGL_TRUE) {
    LOG(WARNING) << " Destroy EGLImage failed.";
  }
  return result;
}

}  // namespace vda

}  // namespace media
