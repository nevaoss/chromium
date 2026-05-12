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

#include "neva/app_runtime/app/app_runtime_visible_region_capture.h"

#include <optional>
#include <vector>

#include "base/base64.h"
#include "base/functional/bind.h"
#include "base/strings/stringprintf.h"
#include "base/task/thread_pool.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_widget_host.h"
#include "content/public/browser/render_widget_host_view.h"
#include "content/public/browser/web_contents.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "ui/gfx/codec/jpeg_codec.h"
#include "ui/gfx/codec/png_codec.h"

namespace neva_app_runtime {

VisibleRegionCapture::VisibleRegionCapture(ReplyCallback callback,
                                           content::WebContents* web_contents,
                                           ImageFormat image_format,
                                           int image_quality,
                                           int output_width,
                                           int output_height,
                                           bool is_transparent)
    : callback_(std::move(callback)),
      image_format_(image_format),
      image_quality_(image_quality),
      is_transparent_(is_transparent) {
  content::RenderWidgetHostView* const view =
      web_contents ? web_contents->GetRenderWidgetHostView() : nullptr;
  if (view && image_format_ != ImageFormat::kNone) {
    output_width = std::max(0, output_width);
    output_height = std::max(0, output_height);

    if (output_width == 0)
      output_height = 0;
    if (output_height == 0)
      output_width = 0;

    view->CopyFromSurface(
        gfx::Rect(), gfx::Size(output_width, output_height),
        base::BindOnce(&VisibleRegionCapture::OnBitmapCaptured,
                       base::Unretained(this)));
    return;
  }
  std::move(callback_).Run(std::string());
}

VisibleRegionCapture::~VisibleRegionCapture() = default;

bool VisibleRegionCapture::EncodeBitmap(const SkBitmap& bitmap,
                                        std::string& base64_data) {
  std::optional<std::vector<uint8_t>> data;
  std::string mime_type;
  if (image_format_ == ImageFormat::kJpeg) {
    data = gfx::JPEGCodec::Encode(bitmap, image_quality_);
    mime_type = "image/jpeg";
  } else if (image_format_ == ImageFormat::kPng) {
    data = gfx::PNGCodec::EncodeBGRASkBitmap(bitmap, !is_transparent_);
    mime_type = "image/png";
  } else {
    NOTREACHED() << "Invalid image format.";
  }

  if (data) {
    std::string_view stream_as_string(
        reinterpret_cast<const char*>(data->data()), data->size());
    base64_data = base::Base64Encode(stream_as_string);
    base64_data.insert(
        0, base::StringPrintf("data:%s;base64,", mime_type.c_str()));
    return true;
  }

  return false;
}

void VisibleRegionCapture::EncodeBitmapOnWorkerThread(
    const SkBitmap& bitmap) {
  std::string base64_data;
  bool success = EncodeBitmap(bitmap, base64_data);
  content::GetUIThreadTaskRunner({})->PostTask(
      FROM_HERE,
      base::BindOnce(&VisibleRegionCapture::OnBitmapEncodedOnUIThread,
                     base::Unretained(this), success, std::move(base64_data)));
}

void VisibleRegionCapture::OnBitmapCaptured(const SkBitmap& bitmap) {
  base::ThreadPool::PostTask(
      FROM_HERE, {base::TaskPriority::USER_VISIBLE},
      base::BindOnce(&VisibleRegionCapture::EncodeBitmapOnWorkerThread,
                     base::Unretained(this),
                     bitmap));
}

void VisibleRegionCapture::OnBitmapEncodedOnUIThread(bool success,
                                                     std::string base64_data) {
  if (success)
    std::move(callback_).Run(std::move(base64_data));
  else
    std::move(callback_).Run(std::string());
}

}  // namespace neva_app_runtime
