// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_PLATFORM_IMAGE_ENCODERS_IMAGE_ENCODER_H_
#define THIRD_PARTY_BLINK_RENDERER_PLATFORM_IMAGE_ENCODERS_IMAGE_ENCODER_H_

#include "base/check_op.h"
#include "base/memory/raw_ptr.h"
#include "third_party/blink/renderer/platform/platform_export.h"
#include "third_party/blink/renderer/platform/wtf/allocator/allocator.h"
#include "third_party/blink/renderer/platform/wtf/vector.h"
#include "third_party/skia/include/core/SkStream.h"
#include "third_party/skia/include/encode/SkJpegEncoder.h"
// TODO(neva_rust): Remove this workaround once Neva supports Rust build.
#if BUILDFLAG(IS_NEVA_SUPPORT_RUST)
#include "third_party/skia/include/encode/SkPngRustEncoder.h"
#else   // !BUILDFLAG(IS_NEVA_SUPPORT_RUST)
#include "third_party/skia/include/encode/SkPngEncoder.h"
#endif  // BUILDFLAG(IS_NEVA_SUPPORT_RUST)
#include "third_party/skia/include/encode/SkWebpEncoder.h"

namespace blink {

class VectorWStream : public SkWStream {
 public:
  VectorWStream(Vector<unsigned char>* dst) : dst_(dst) {
    DCHECK(dst_);
    DCHECK_EQ(0UL, dst->size());
  }

  bool write(const void* buffer, size_t size) override {
    DCHECK_LE(size, std::numeric_limits<wtf_size_t>::max());
    // SAFETY: Skia encoders guarantees `buffer` and `size` are safe.
    dst_->append_range(UNSAFE_BUFFERS(
        base::span(reinterpret_cast<const unsigned char*>(buffer), size)));
    return true;
  }

  size_t bytesWritten() const override { return dst_->size(); }

 private:
  // Does not have ownership.
  raw_ptr<Vector<unsigned char>> dst_;
};

enum ImageEncodingMimeType {
  kMimeTypePng,
  kMimeTypeJpeg,
  kMimeTypeWebp,
};

class PLATFORM_EXPORT ImageEncoder {
  USING_FAST_MALLOC(ImageEncoder);

 public:
  static bool Encode(Vector<unsigned char>* dst,
                     const SkPixmap& src,
                     const SkJpegEncoder::Options&);

  // TODO(neva_rust): Remove this workaround once Neva supports Rust build.
#if BUILDFLAG(IS_NEVA_SUPPORT_RUST)
  static bool Encode(Vector<unsigned char>* dst,
                     const SkPixmap& src,
                     SkPngRustEncoder::CompressionLevel);
#else   // !BUILDFLAG(IS_NEVA_SUPPORT_RUST)
  static bool Encode(Vector<unsigned char>* dst,
                     const SkPixmap& src,
                     const SkPngEncoder::Options&);
#endif  // BUILDFLAG(IS_NEVA_SUPPORT_RUST)

  static bool Encode(Vector<unsigned char>* dst,
                     const SkPixmap& src,
                     const SkWebpEncoder::Options&);

  static bool Encode(Vector<unsigned char>* dst,
                     const SkPixmap& src,
                     ImageEncodingMimeType mime_type,
                     double quality);

  static int MaxDimension(ImageEncodingMimeType mime_type);

  static std::unique_ptr<ImageEncoder> Create(Vector<unsigned char>* dst,
                                              const SkPixmap& src,
                                              const SkJpegEncoder::Options&);

  // TODO(neva_rust): Remove this workaround once Neva supports Rust build.
#if !BUILDFLAG(IS_NEVA_SUPPORT_RUST)
  static std::unique_ptr<ImageEncoder> Create(Vector<unsigned char>* dst,
                                              const SkPixmap& src,
                                              const SkPngEncoder::Options&);
#else   // BUILDFLAG(IS_NEVA_SUPPORT_RUST)
  static std::unique_ptr<ImageEncoder> Create(
      Vector<unsigned char>* dst,
      const SkPixmap& src,
      SkPngRustEncoder::CompressionLevel);
#endif  // !BUILDFLAG(IS_NEVA_SUPPORT_RUST)

  bool encodeRows(int numRows) { return encoder_->encodeRows(numRows); }

  /**
   *  If quality is in [0, 1], this will simply convert to a [0, 100]
   *  integer scale (which is what is used by libjpeg-turbo).
   *
   *  Otherwise, this will return the default value (92).
   */
  static int ComputeJpegQuality(double quality);

  /**
   *  Sets Skia encoding options based on the requested quality.
   *
   *  If quality is 1, this will signal a lossless encode.
   *
   *  Otherwise, this will use webp lossy encoding.
   *  If quality is in [0, 1), this will simply convert to a [0, 100)
   *  float scale (which is what is used by libwebp).  If the quality
   *  is out of range, this will perform a lossy encode with the default
   *  value (80).
   */
  static SkWebpEncoder::Options ComputeWebpOptions(double quality);

 private:
  ImageEncoder(Vector<unsigned char>* dst) : dst_(dst) {}

  VectorWStream dst_;
  std::unique_ptr<SkEncoder> encoder_;
};
}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_PLATFORM_IMAGE_ENCODERS_IMAGE_ENCODER_H_
