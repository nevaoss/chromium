// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MEMORY_RAW_PTR_H_
#define BASE_MEMORY_RAW_PTR_H_

// Although `raw_ptr` is part of the standalone PA distribution, it is
// easier to use the shorter path in `//base/memory`. We retain this
// facade header for ease of typing.
// TODO(neva): For unkwown reason WAM build requires full paths of
// partition_alloc. Need to investigate the issue and provide more proper
// solution.
///@name USE_NEVA_APPRUNTIME
///@{
#include "base/allocator/partition_allocator/src/partition_alloc/pointers/raw_ptr.h"  // IWYU pragma: export
/*
///@}
#include "partition_alloc/pointers/raw_ptr.h"  // IWYU pragma: export
///@name USE_NEVA_APPRUNTIME
///@{
*/
///@}

#endif  // BASE_MEMORY_RAW_PTR_H_
