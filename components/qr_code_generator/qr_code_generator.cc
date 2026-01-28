// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/qr_code_generator/qr_code_generator.h"

#include <utility>
#include <vector>

#include "base/check_op.h"
// TODO(neva_rust): Remove this workaround once Neva supports Rust build.
#if BUILDFLAG(IS_NEVA_SUPPORT_RUST)
#include "base/containers/span_rust.h"
#endif  // BUILDFLAG(IS_NEVA_SUPPORT_RUST)
#include "base/numerics/safe_conversions.h"
// TODO(neva_rust): Remove this workaround once Neva supports Rust build.
#if BUILDFLAG(IS_NEVA_SUPPORT_RUST)
#include "components/qr_code_generator/qr_code_generator_ffi_glue.rs.h"
#endif  // BUILDFLAG(IS_NEVA_SUPPORT_RUST)

namespace qr_code_generator {

GeneratedCode::GeneratedCode() = default;
GeneratedCode::~GeneratedCode() = default;
GeneratedCode::GeneratedCode(GeneratedCode&&) = default;
GeneratedCode& GeneratedCode::operator=(GeneratedCode&&) = default;

base::expected<GeneratedCode, Error> GenerateCode(
    base::span<const uint8_t> in,
    std::optional<int> min_version) {
// TODO(neva_rust): Remove this workaround once Neva supports Rust build.
#if BUILDFLAG(IS_NEVA_SUPPORT_RUST)
  rust::Slice<const uint8_t> rs_in = base::SpanToRustSlice(in);

  // `min_version` might come from a fuzzer and therefore we use a lenient
  // `saturated_cast` instead of a `checked_cast`.
  int16_t rs_min_version =
      base::saturated_cast<int16_t>(min_version.value_or(0));

  std::vector<uint8_t> result_pixels;
  size_t result_width = 0;
  Error result_error = Error::kUnknownError;
  bool result_is_success = generate_qr_code_using_rust(
      rs_in, rs_min_version, result_pixels, result_width, result_error);

  if (!result_is_success) {
    return base::unexpected(result_error);
  }
  GeneratedCode code;
  code.data = std::move(result_pixels);
  code.qr_size = base::checked_cast<int>(result_width);
  CHECK_EQ(code.data.size(), static_cast<size_t>(code.qr_size * code.qr_size));
  return code;
#else  // !BUILDFLAG(IS_NEVA_SUPPORT_RUST)
  GeneratedCode code;
  return code;
#endif  // BUILDFLAG(IS_NEVA_SUPPORT_RUST)
}

}  // namespace qr_code_generator
