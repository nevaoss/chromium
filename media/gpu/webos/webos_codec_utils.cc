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

#include "media/gpu/webos/webos_codec_utils.h"

#include "base/logging.h"

#include <libdrm/drm_fourcc.h>

namespace media {

uint32_t DrmPixelFormatFrom(mcil::VideoPixelFormat format) {
  switch (format) {
    case mcil::PIXEL_FORMAT_NV12:
      return DRM_FORMAT_NV12;
    case mcil::PIXEL_FORMAT_I420:
      return DRM_FORMAT_YUV420;
    case mcil::PIXEL_FORMAT_YV12:
      return DRM_FORMAT_YVU420;
    case mcil::PIXEL_FORMAT_BGRA:
      return DRM_FORMAT_ARGB8888;
    default:
      break;
  }
  return 0;
}

mcil::VideoPixelFormat MCILVideoPixelFormatFrom(VideoPixelFormat pixel_format) {
  switch (pixel_format) {
    case PIXEL_FORMAT_I420:
      return mcil::PIXEL_FORMAT_I420;
    case PIXEL_FORMAT_YV12:
      return mcil::PIXEL_FORMAT_YV12;
    case PIXEL_FORMAT_I422:
      return mcil::PIXEL_FORMAT_I422;
    case PIXEL_FORMAT_I420A:
      return mcil::PIXEL_FORMAT_I420A;
    case PIXEL_FORMAT_I444:
      return mcil::PIXEL_FORMAT_I444;
    case PIXEL_FORMAT_NV12:
      return mcil::PIXEL_FORMAT_NV12;
    case PIXEL_FORMAT_NV21:
      return mcil::PIXEL_FORMAT_NV21;
    case PIXEL_FORMAT_UYVY:
      return mcil::PIXEL_FORMAT_UYVY;
    case PIXEL_FORMAT_YUY2:
      return mcil::PIXEL_FORMAT_YUY2;
    case PIXEL_FORMAT_ARGB:
      return mcil::PIXEL_FORMAT_ARGB;
    case PIXEL_FORMAT_XRGB:
      return mcil::PIXEL_FORMAT_XRGB;
    case PIXEL_FORMAT_RGB24:
      return mcil::PIXEL_FORMAT_RGB24;
    case PIXEL_FORMAT_MJPEG:
      return mcil::PIXEL_FORMAT_MJPEG;
    case PIXEL_FORMAT_YUV420P9:
      return mcil::PIXEL_FORMAT_YUV420P9;
    case PIXEL_FORMAT_YUV420P10:
      return mcil::PIXEL_FORMAT_YUV420P10;
    case PIXEL_FORMAT_YUV422P9:
      return mcil::PIXEL_FORMAT_YUV422P9;
    case PIXEL_FORMAT_YUV422P10:
      return mcil::PIXEL_FORMAT_YUV422P10;
    case PIXEL_FORMAT_YUV444P9:
      return mcil::PIXEL_FORMAT_YUV444P9;
    case PIXEL_FORMAT_YUV444P10:
      return mcil::PIXEL_FORMAT_YUV444P10;
    case PIXEL_FORMAT_YUV420P12:
      return mcil::PIXEL_FORMAT_YUV420P12;
    case PIXEL_FORMAT_YUV422P12:
      return mcil::PIXEL_FORMAT_YUV422P12;
    case PIXEL_FORMAT_YUV444P12:
      return mcil::PIXEL_FORMAT_YUV444P12;
    case PIXEL_FORMAT_Y16:
      return mcil::PIXEL_FORMAT_Y16;
    case PIXEL_FORMAT_ABGR:
      return mcil::PIXEL_FORMAT_ABGR;
    case PIXEL_FORMAT_XBGR:
      return mcil::PIXEL_FORMAT_XBGR;
    case PIXEL_FORMAT_P010LE:
      return mcil::PIXEL_FORMAT_P016LE;
    case PIXEL_FORMAT_XR30:
      return mcil::PIXEL_FORMAT_XR30;
    case PIXEL_FORMAT_XB30:
      return mcil::PIXEL_FORMAT_XB30;
    case PIXEL_FORMAT_BGRA:
      return mcil::PIXEL_FORMAT_BGRA;
    case PIXEL_FORMAT_RGBAF16:
      return mcil::PIXEL_FORMAT_RGBAF16;
    case PIXEL_FORMAT_I422A:
    case PIXEL_FORMAT_I444A:
    case PIXEL_FORMAT_YUV420AP10:
    case PIXEL_FORMAT_YUV422AP10:
    case PIXEL_FORMAT_YUV444AP10:
    default:
      break;
  }
  return mcil::PIXEL_FORMAT_UNKNOWN;
}

VideoPixelFormat VideoPixelFormatFrom(mcil::VideoPixelFormat pixel_format) {
  switch (pixel_format) {
    case mcil::PIXEL_FORMAT_I420:
      return PIXEL_FORMAT_I420;
    case mcil::PIXEL_FORMAT_YV12:
      return PIXEL_FORMAT_YV12;
    case mcil::PIXEL_FORMAT_I422:
      return PIXEL_FORMAT_I422;
    case mcil::PIXEL_FORMAT_I420A:
      return PIXEL_FORMAT_I420A;
    case mcil::PIXEL_FORMAT_I444:
      return PIXEL_FORMAT_I444;
    case mcil::PIXEL_FORMAT_NV12:
      return PIXEL_FORMAT_NV12;
    case mcil::PIXEL_FORMAT_NV21:
      return PIXEL_FORMAT_NV21;
    case mcil::PIXEL_FORMAT_UYVY:
      return PIXEL_FORMAT_UYVY;
    case mcil::PIXEL_FORMAT_YUY2:
      return PIXEL_FORMAT_YUY2;
    case mcil::PIXEL_FORMAT_ARGB:
      return PIXEL_FORMAT_ARGB;
    case mcil::PIXEL_FORMAT_XRGB:
      return PIXEL_FORMAT_XRGB;
    case mcil::PIXEL_FORMAT_RGB24:
      return PIXEL_FORMAT_RGB24;
    case mcil::PIXEL_FORMAT_MJPEG:
      return PIXEL_FORMAT_MJPEG;
    case mcil::PIXEL_FORMAT_YUV420P9:
      return PIXEL_FORMAT_YUV420P9;
    case mcil::PIXEL_FORMAT_YUV420P10:
      return PIXEL_FORMAT_YUV420P10;
    case mcil::PIXEL_FORMAT_YUV422P9:
      return PIXEL_FORMAT_YUV422P9;
    case mcil::PIXEL_FORMAT_YUV422P10:
      return PIXEL_FORMAT_YUV422P10;
    case mcil::PIXEL_FORMAT_YUV444P9:
      return PIXEL_FORMAT_YUV444P9;
    case mcil::PIXEL_FORMAT_YUV444P10:
      return PIXEL_FORMAT_YUV444P10;
    case mcil::PIXEL_FORMAT_YUV420P12:
      return PIXEL_FORMAT_YUV420P12;
    case mcil::PIXEL_FORMAT_YUV422P12:
      return PIXEL_FORMAT_YUV422P12;
    case mcil::PIXEL_FORMAT_YUV444P12:
      return PIXEL_FORMAT_YUV444P12;
    case mcil::PIXEL_FORMAT_Y16:
      return PIXEL_FORMAT_Y16;
    case mcil::PIXEL_FORMAT_ABGR:
      return PIXEL_FORMAT_ABGR;
    case mcil::PIXEL_FORMAT_XBGR:
      return PIXEL_FORMAT_XBGR;
    case mcil::PIXEL_FORMAT_P016LE:
      return PIXEL_FORMAT_P010LE;
    case mcil::PIXEL_FORMAT_XR30:
      return PIXEL_FORMAT_XR30;
    case mcil::PIXEL_FORMAT_XB30:
      return PIXEL_FORMAT_XB30;
    case mcil::PIXEL_FORMAT_BGRA:
      return PIXEL_FORMAT_BGRA;
    case mcil::PIXEL_FORMAT_RGBAF16:
      return PIXEL_FORMAT_RGBAF16;
    default:
      break;
  }
  return PIXEL_FORMAT_UNKNOWN;
}

mcil::VideoCodecProfile MCILVideoCodecProfileFrom(VideoCodecProfile profile) {
  switch (profile) {
    case H264PROFILE_BASELINE:
      return mcil::H264PROFILE_BASELINE;
    case H264PROFILE_MAIN:
      return mcil::H264PROFILE_MAIN;
    case H264PROFILE_EXTENDED:
      return mcil::H264PROFILE_EXTENDED;
    case H264PROFILE_HIGH:
      return mcil::H264PROFILE_HIGH;
    case H264PROFILE_HIGH10PROFILE:
      return mcil::H264PROFILE_HIGH10PROFILE;
    case H264PROFILE_HIGH422PROFILE:
      return mcil::H264PROFILE_HIGH422PROFILE;
    case H264PROFILE_HIGH444PREDICTIVEPROFILE:
      return mcil::H264PROFILE_HIGH444PREDICTIVEPROFILE;
    case H264PROFILE_SCALABLEBASELINE:
      return mcil::H264PROFILE_SCALABLEBASELINE;
    case H264PROFILE_SCALABLEHIGH:
      return mcil::H264PROFILE_SCALABLEHIGH;
    case H264PROFILE_STEREOHIGH:
      return mcil::H264PROFILE_STEREOHIGH;
    case H264PROFILE_MULTIVIEWHIGH:
      return mcil::H264PROFILE_MULTIVIEWHIGH;
    case VP8PROFILE_ANY:
      return mcil::VP8PROFILE_ANY;
    case VP9PROFILE_PROFILE0:
      return mcil::VP9PROFILE_PROFILE0;
    case VP9PROFILE_PROFILE1:
      return mcil::VP9PROFILE_PROFILE1;
    case VP9PROFILE_PROFILE2:
      return mcil::VP9PROFILE_PROFILE2;
    case VP9PROFILE_PROFILE3:
      return mcil::VP9PROFILE_PROFILE3;
    case HEVCPROFILE_MAIN:
      return mcil::HEVCPROFILE_MAIN;
    case HEVCPROFILE_MAIN10:
      return mcil::HEVCPROFILE_MAIN10;
    case HEVCPROFILE_MAIN_STILL_PICTURE:
      return mcil::HEVCPROFILE_MAIN_STILL_PICTURE;
    case DOLBYVISION_PROFILE0:
      return mcil::DOLBYVISION_PROFILE0;
    case DOLBYVISION_PROFILE5:
      return mcil::DOLBYVISION_PROFILE5;
    case DOLBYVISION_PROFILE7:
      return mcil::DOLBYVISION_PROFILE7;
    case THEORAPROFILE_ANY:
      return mcil::THEORAPROFILE_ANY;
    case AV1PROFILE_PROFILE_MAIN:
      return mcil::AV1PROFILE_PROFILE_MAIN;
    case AV1PROFILE_PROFILE_HIGH:
      return mcil::AV1PROFILE_PROFILE_HIGH;
    case AV1PROFILE_PROFILE_PRO:
      return mcil::AV1PROFILE_PROFILE_PRO;
    case DOLBYVISION_PROFILE8:
      return mcil::DOLBYVISION_PROFILE8;
    case DOLBYVISION_PROFILE9:
      return mcil::DOLBYVISION_PROFILE9;
    case HEVCPROFILE_REXT:
    case HEVCPROFILE_HIGH_THROUGHPUT:
    case HEVCPROFILE_MULTIVIEW_MAIN:
    case HEVCPROFILE_SCALABLE_MAIN:
    case HEVCPROFILE_3D_MAIN:
    case HEVCPROFILE_SCREEN_EXTENDED:
    case HEVCPROFILE_SCALABLE_REXT:
    case HEVCPROFILE_HIGH_THROUGHPUT_SCREEN_EXTENDED:
    default:
      break;
  }
  return mcil::VIDEO_CODEC_PROFILE_UNKNOWN;
}

VideoCodecProfile VideoCodecProfileFrom(mcil::VideoCodecProfile profile) {
  switch (profile) {
    case mcil::H264PROFILE_BASELINE:
      return H264PROFILE_BASELINE;
    case mcil::H264PROFILE_MAIN:
      return H264PROFILE_MAIN;
    case mcil::H264PROFILE_EXTENDED:
      return H264PROFILE_EXTENDED;
    case mcil::H264PROFILE_HIGH:
      return H264PROFILE_HIGH;
    case mcil::H264PROFILE_HIGH10PROFILE:
      return H264PROFILE_HIGH10PROFILE;
    case mcil::H264PROFILE_HIGH422PROFILE:
      return H264PROFILE_HIGH422PROFILE;
    case mcil::H264PROFILE_HIGH444PREDICTIVEPROFILE:
      return H264PROFILE_HIGH444PREDICTIVEPROFILE;
    case mcil::H264PROFILE_SCALABLEBASELINE:
      return H264PROFILE_SCALABLEBASELINE;
    case mcil::H264PROFILE_SCALABLEHIGH:
      return H264PROFILE_SCALABLEHIGH;
    case mcil::H264PROFILE_STEREOHIGH:
      return H264PROFILE_STEREOHIGH;
    case mcil::H264PROFILE_MULTIVIEWHIGH:
      return H264PROFILE_MULTIVIEWHIGH;
    case mcil::VP8PROFILE_ANY:
      return VP8PROFILE_ANY;
    case mcil::VP9PROFILE_PROFILE0:
      return VP9PROFILE_PROFILE0;
    case mcil::VP9PROFILE_PROFILE1:
      return VP9PROFILE_PROFILE1;
    case mcil::VP9PROFILE_PROFILE2:
      return VP9PROFILE_PROFILE2;
    case mcil::VP9PROFILE_PROFILE3:
      return VP9PROFILE_PROFILE3;
    case mcil::HEVCPROFILE_MAIN:
      return HEVCPROFILE_MAIN;
    case mcil::HEVCPROFILE_MAIN10:
      return HEVCPROFILE_MAIN10;
    case mcil::HEVCPROFILE_MAIN_STILL_PICTURE:
      return HEVCPROFILE_MAIN_STILL_PICTURE;
    case mcil::DOLBYVISION_PROFILE0:
      return DOLBYVISION_PROFILE0;
    case mcil::DOLBYVISION_PROFILE5:
      return DOLBYVISION_PROFILE5;
    case mcil::DOLBYVISION_PROFILE7:
      return DOLBYVISION_PROFILE7;
    case mcil::THEORAPROFILE_ANY:
      return THEORAPROFILE_ANY;
    case mcil::AV1PROFILE_PROFILE_MAIN:
      return AV1PROFILE_PROFILE_MAIN;
    case mcil::AV1PROFILE_PROFILE_HIGH:
      return AV1PROFILE_PROFILE_HIGH;
    case mcil::AV1PROFILE_PROFILE_PRO:
      return AV1PROFILE_PROFILE_PRO;
    case mcil::DOLBYVISION_PROFILE8:
      return DOLBYVISION_PROFILE8;
    case mcil::DOLBYVISION_PROFILE9:
      return DOLBYVISION_PROFILE9;
    default:
      break;
  }
  return VIDEO_CODEC_PROFILE_UNKNOWN;
}

absl::optional<VideoFrameLayout> VideoFrameLayoutFrom(
    mcil::scoped_refptr<mcil::VideoFrame> video_frame) {
  VLOG(2) << __func__;

  if (!video_frame)
    return absl::nullopt;

  const size_t num_color_planes = video_frame->color_planes.size();
  const VideoPixelFormat video_format =
      VideoPixelFormatFrom(video_frame->format);

  std::vector<ColorPlaneLayout> planes;
  planes.reserve(num_color_planes);
  for (size_t i = 0; i < num_color_planes; ++i) {
    const mcil::ColorPlane color_plane = video_frame->color_planes[i];
    planes.emplace_back(color_plane.stride, color_plane.offset,
                        color_plane.size);
  }

  gfx::Size coded_size(video_frame->coded_size.width,
                       video_frame->coded_size.height);
  constexpr size_t buffer_alignment = 0x1000;
  if (!video_frame->is_multi_planar) {
    return VideoFrameLayout::CreateWithPlanes(
        video_format, coded_size, std::move(planes), buffer_alignment);
  } else {
    return VideoFrameLayout::CreateMultiPlanar(
        video_format, coded_size, std::move(planes), buffer_alignment);
  }
}

}  // namespace media
