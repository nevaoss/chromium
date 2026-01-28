// Copyright 2025 LG Electronics, Inc.
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

// This file is originally from removed
// content/common/user_level_memory_pressure_signal_features.h

// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_COMMON_NEVA_USER_LEVEL_MEMORY_PRESSURE_SIGNAL_FEATURES_H_
#define CONTENT_COMMON_NEVA_USER_LEVEL_MEMORY_PRESSURE_SIGNAL_FEATURES_H_

#include "base/feature_list.h"
#include "base/time/time.h"
#include "content/common/content_export.h"

namespace content::features {
CONTENT_EXPORT BASE_DECLARE_FEATURE(kUserLevelMemoryPressureSignal);
CONTENT_EXPORT bool IsUserLevelMemoryPressureSignalEnabled();
CONTENT_EXPORT base::TimeDelta InertInterval();
CONTENT_EXPORT base::TimeDelta MinUserMemoryPressureInterval();
}  // namespace content::features

#endif  // CONTENT_COMMON_NEVA_USER_LEVEL_MEMORY_PRESSURE_SIGNAL_FEATURES_H_
