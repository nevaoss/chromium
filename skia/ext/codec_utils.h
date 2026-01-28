// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKIA_EXT_CODEC_UTILS_H_
#define SKIA_EXT_CODEC_UTILS_H_

#include <string>

// NOTE(neva): Required for usage of IS_NEVA_APPRUNTIME build flag.
///@name IS_NEVA_APPRUNTIME
///@{
#include "build/build_config.h"
///@}

#include "third_party/skia/include/core/SkRefCnt.h"

class GrDirectContext;
class SkData;
class SkImage;
class SkPixmap;

namespace skia {

SK_API sk_sp<SkData> EncodePngAsSkData(const SkPixmap& src);
SK_API sk_sp<SkData> EncodePngAsSkData(GrDirectContext* context,
                                       const SkImage* src);
// TODO(neva_rust): Remove this workaround once Neva supports Rust build.
#if !BUILDFLAG(IS_NEVA_SUPPORT_RUST)
SK_API sk_sp<SkData> EncodePngAsSkData(GrDirectContext* context,
                                       const SkImage* src,
                                       int zlib_compression_level);
#else   // !BUILDFLAG(IS_NEVA_SUPPORT_RUST)
SK_API sk_sp<SkData> FastEncodePngAsSkData(GrDirectContext* context,
                                           const SkImage* src);
#endif  // BUILDFLAG(IS_NEVA_SUPPORT_RUST)
SK_API std::string EncodePngAsDataUri(const SkPixmap& src);

// This is not thread safe and should only be called via startup
SK_API void EnsurePNGDecoderRegistered();

}  // namespace skia

#endif  // SKIA_EXT_CODEC_UTILS_H_
