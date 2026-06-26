// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CC_TILES_IMAGE_DECODE_CACHE_UTILS_CC_
#define CC_TILES_IMAGE_DECODE_CACHE_UTILS_CC_

#include "cc/tiles/image_decode_cache_utils.h"

#include "base/byte_size.h"
#include "build/build_config.h"

#if !BUILDFLAG(IS_ANDROID)
#include "base/system/sys_info.h"
#endif

#if BUILDFLAG(IS_NEVA_APPRUNTIME)
#include "base/command_line.h"
#include "base/neva/base_switches.h"
#include "base/strings/string_number_conversions.h"
#endif // BUILDFLAG(IS_NEVA_APPRUNTIME)

namespace cc {

// static
size_t ImageDecodeCacheUtils::GetWorkingSetBytesForImageDecode(
    bool for_renderer) {
  base::ByteSize decoded_image_working_set_budget = base::MiBU(128);
#if !BUILDFLAG(IS_ANDROID)
  if (for_renderer) {
    const bool using_low_memory_policy = base::SysInfo::IsLowEndDevice();
    // If there's over 4GB of RAM, increase the working set size to 256MB for
    // both gpu and software.
    constexpr base::ByteSize kImageDecodeMemoryThreshold = base::GiBU(4);
    if (using_low_memory_policy) {
      decoded_image_working_set_budget = base::MiBU(32);
    } else if (base::SysInfo::AmountOfTotalPhysicalMemory() >=
               kImageDecodeMemoryThreshold) {
      decoded_image_working_set_budget = base::MiBU(256);
    }
#if BUILDFLAG(IS_NEVA_APPRUNTIME)
    const base::CommandLine& cmd = *base::CommandLine::ForCurrentProcess();
    if (cmd.HasSwitch(::switches::kDecodedImageWorkingSetBudgetMB)) {
      int budget_bytes_mb = 0;
      if (base::StringToInt(cmd.GetSwitchValueASCII(
                                ::switches::kDecodedImageWorkingSetBudgetMB),
                            &budget_bytes_mb)) {
        base::ByteCount budget_bytes = base::MiB(budget_bytes_mb);
        if (!using_low_memory_policy ||
            budget_bytes < decoded_image_working_set_budget) {
          decoded_image_working_set_budget = budget_bytes;
        }
      }
    }
#endif  // BUILDFLAG(IS_NEVA_APPRUNTIME)
  }
#endif  // !BUILDFLAG(IS_ANDROID)
  return decoded_image_working_set_budget.InBytes();
}

}  // namespace cc

#endif  // CC_TILES_IMAGE_DECODE_CACHE_UTILS_CC_
