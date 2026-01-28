// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "skia/ext/codec_utils.h"

#include "base/base64.h"
#include "skia/ext/skia_utils_base.h"
#include "third_party/skia/include/codec/SkCodec.h"
// TODO(neva_rust): Remove this workaround once Neva supports Rust build.
#if BUILDFLAG(IS_NEVA_SUPPORT_RUST)
#include "third_party/skia/include/codec/SkPngRustDecoder.h"
#else   // !BUILDFLAG(IS_NEVA_SUPPORT_RUST)
#include "base/check.h"
#include "third_party/skia/include/codec/SkPngDecoder.h"
#endif  // BUILDFLAG(IS_NEVA_SUPPORT_RUST)
#include "third_party/skia/include/core/SkData.h"
// TODO(neva_rust): Remove this workaround once Neva supports Rust build.
#if BUILDFLAG(IS_NEVA_SUPPORT_RUST)
#include "third_party/skia/include/encode/SkPngRustEncoder.h"
#else   // !BUILDFLAG(IS_NEVA_SUPPORT_RUST)
#include "third_party/skia/include/core/SkImage.h"
#include "third_party/skia/include/core/SkPixmap.h"
#include "third_party/skia/include/core/SkStream.h"
#include "third_party/skia/include/encode/SkPngEncoder.h"
#endif  // BUILDFLAG(IS_NEVA_SUPPORT_RUST)

namespace skia {

namespace {

// TODO(neva_rust): Remove this workaround once Neva supports Rust build.
#if !BUILDFLAG(IS_NEVA_SUPPORT_RUST)
sk_sp<SkData> EncodePngAsSkData(const SkPixmap& src,
                                const SkPngEncoder::Options& options) {
  SkDynamicMemoryWStream stream;
  if (!SkPngEncoder::Encode(&stream, src, options)) {
    return nullptr;
  }
  return stream.detachAsData();
}
#else   // BUILDFLAG(IS_NEVA_SUPPORT_RUST)
sk_sp<SkData> EncodePngAsSkData(
    GrDirectContext* context,
    const SkImage* src,
    SkPngRustEncoder::CompressionLevel compression_level) {
  const SkPngRustEncoder::Options options = {.fCompressionLevel =
                                                 compression_level};
  return SkPngRustEncoder::Encode(context, src, options);
}
#endif  // !BUILDFLAG(IS_NEVA_SUPPORT_RUST)

}  // namespace

// TODO(neva_rust) This func is used in
// third_party/blink/renderer/platform/graphics/canvas_hibernation_handler.cc.
// Supporting legacy way to get sk_sp<SkData> encoded_uncompressed instead of
// using skia::FastEncodePngAsSkData which uses Rust-based PNG encoder.
// Remove this workaround once Neva supports Rust build.
#if !BUILDFLAG(IS_NEVA_SUPPORT_RUST)
sk_sp<SkData> EncodePngAsSkData(GrDirectContext* context,
                                const SkImage* src,
                                int zlib_compression_level) {
  if (!src) {
    return nullptr;
  }

  sk_sp<SkImage> raster_image = src->makeRasterImage(context);
  if (!raster_image) {
    return nullptr;
  }

  SkPixmap pixmap;
  bool success = raster_image->peekPixels(&pixmap);

  // `peekPixels` should always succeed for raster images.
  CHECK(success);

  const SkPngEncoder::Options options = {.fZLibLevel = zlib_compression_level};
  return EncodePngAsSkData(pixmap, options);
}
#endif  // !BUILDFLAG(IS_NEVA_SUPPORT_RUST)

sk_sp<SkData> EncodePngAsSkData(const SkPixmap& src) {
// TODO(neva_rust): Remove this workaround once Neva supports Rust build.
#if !BUILDFLAG(IS_NEVA_SUPPORT_RUST)
  const SkPngEncoder::Options kDefaultOptions = {};
  return EncodePngAsSkData(src, kDefaultOptions);
#else   // BUILDFLAG(IS_NEVA_SUPPORT_RUST)
  const SkPngRustEncoder::Options kDefaultOptions = {};
  return SkPngRustEncoder::Encode(src, kDefaultOptions);
#endif  // !BUILDFLAG(IS_NEVA_SUPPORT_RUST)
}

// TODO(neva_rust): Remove this workaround once Neva supports Rust build.
#if !BUILDFLAG(IS_NEVA_SUPPORT_RUST)
sk_sp<SkData> EncodePngAsSkData(GrDirectContext* context, const SkImage* src) {
  // This is the default level in
  // `third_party/skia/include/encode/SkPngEncoder.h`.
  const int kDefaultZlibCompressionLevel = 6;

  return EncodePngAsSkData(context, src, kDefaultZlibCompressionLevel);
}
#else   // BUILDFLAG(IS_NEVA_SUPPORT_RUST)
sk_sp<SkData> EncodePngAsSkData(GrDirectContext* context, const SkImage* src) {
  return EncodePngAsSkData(context, src,
                           SkPngRustEncoder::CompressionLevel::kMedium);
}
#endif  // !BUILDFLAG(IS_NEVA_SUPPORT_RUST)

// TODO(neva_rust): Remove this workaround once Neva supports Rust build.
#if BUILDFLAG(IS_NEVA_SUPPORT_RUST)
sk_sp<SkData> FastEncodePngAsSkData(GrDirectContext* context,
                                    const SkImage* src) {
  return EncodePngAsSkData(context, src,
                           SkPngRustEncoder::CompressionLevel::kLow);
}
#endif  // BUILDFLAG(IS_NEVA_SUPPORT_RUST)

std::string EncodePngAsDataUri(const SkPixmap& src) {
  std::string result;
  if (sk_sp<SkData> data = EncodePngAsSkData(src); data) {
    result += "data:image/png;base64,";
    result += base::Base64Encode(skia::as_byte_span(*data));
  }
  return result;
}

void EnsurePNGDecoderRegistered() {
// TODO(neva_rust): Remove this workaround once Neva supports Rust build.
#if BUILDFLAG(IS_NEVA_SUPPORT_RUST)
  SkCodecs::Register(SkPngRustDecoder::Decoder());
#else   // !BUILDFLAG(IS_NEVA_SUPPORT_RUST)
  SkCodecs::Register(SkPngDecoder::Decoder());
#endif  // BUILDFLAG(IS_NEVA_SUPPORT_RUST)
}

}  // namespace skia
