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

#ifndef NEVA_APP_RUNTIME_APP_APP_RUNTIME_VISIBLE_REGION_CAPTURE_H_
#define NEVA_APP_RUNTIME_APP_APP_RUNTIME_VISIBLE_REGION_CAPTURE_H_

#include <string>

#include "base/functional/callback.h"
#include "base/memory/scoped_refptr.h"

class SkBitmap;

namespace base {
class TaskRunner;
}  // namespace base

namespace content {
class WebContents;
}  // namespace content

namespace neva_app_runtime {

class VisibleRegionCapture {
 public:
  enum class ImageFormat { kNone, kJpeg, kPng };
  using ReplyCallback = base::OnceCallback<void(std::string)>;

  VisibleRegionCapture(ReplyCallback callback,
                       content::WebContents* web_contents,
                       ImageFormat format = ImageFormat::kJpeg,
                       int quality = 90,
                       int output_width = 0,
                       int output_heitgh = 0,
                       bool is_transparent = false);

  ~VisibleRegionCapture();
  VisibleRegionCapture(const VisibleRegionCapture&) = delete;
  VisibleRegionCapture& operator=(const VisibleRegionCapture&) = delete;

 private:
  bool EncodeBitmap(const SkBitmap& bitmap, std::string& base64_data);
  void EncodeBitmapOnWorkerThread(const SkBitmap& bitmap);
  void OnBitmapCaptured(const SkBitmap& bitmap);
  void OnBitmapEncodedOnUIThread(bool success, std::string base64_data);

  ReplyCallback callback_;
  ImageFormat image_format_;
  int image_quality_;
  bool is_transparent_;
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_APP_APP_RUNTIME_VISIBLE_REGION_CAPTURE_H_
