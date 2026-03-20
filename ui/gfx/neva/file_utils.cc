// Copyright 2021 LG Electronics, Inc.
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

#include "base/containers/span.h"
#include "base/logging.h"
#include "base/memory/ref_counted_memory.h"
#include "base/memory/scoped_refptr.h"
#include "ui/gfx/codec/png_codec.h"
#include "ui/gfx/neva/file_utils.h"

namespace gfx {

SkBitmap* DecodeSkBitmapFromPNG(const base::FilePath& path) {
  if (path.empty())
    return nullptr;

  base::File file(path, base::File::FLAG_OPEN | base::File::FLAG_READ);
  if (!file.IsValid())
    return nullptr;

  scoped_refptr<base::RefCountedBytes> memory;

  int64_t length = file.GetLength();
  if (length > 0 && length < INT_MAX) {
    int size = static_cast<int>(length);
    std::vector<unsigned char> raw_data(size);

    std::optional<size_t> data =
        file.ReadAtCurrentPos(base::as_writable_byte_span(raw_data));

    if (data.has_value() && *data == data) {
      memory = base::MakeRefCounted<base::RefCountedBytes>(std::move(raw_data));
    }
  } else {
    return nullptr;
  }

  if (!memory.get()) {
    LOG(ERROR) << "Unable to read file path = " << path;
    return nullptr;
  }

  SkBitmap bitmap = gfx::PNGCodec::Decode(*memory);
  if (bitmap.isNull()) {
    LOG(ERROR) << "Unable to decode image path = " << path;
    return nullptr;
  }

  return new SkBitmap(bitmap);
}

}  // namespace gfx
