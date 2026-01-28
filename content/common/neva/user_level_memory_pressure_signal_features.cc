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
// content/common/user_level_memory_pressure_signal_features.cc

// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/common/neva/user_level_memory_pressure_signal_features.h"

#include "base/metrics/field_trial_params.h"
#include "base/system/sys_info.h"

namespace content::features {

namespace {

constexpr base::TimeDelta kDefaultMinimumInterval = base::Minutes(10);

// Each renderer does not generate memory pressure signals until the interval
// has passed after page loading is finished. This parameter must be larger
// than or equal to the time from navigation start to the time the
// DOMContentLoaded event is finished. 5min is much larger than
// the 99p of PageLoad.DocumentTiming.NavigationToDOMContentLoadedEventFired
// (14sec) and we expect the DOMContentLoaded events will finish in 5min.
// Negative inert interval disables delayed memory pressure signals
// This is intended to keep the old behavior.
constexpr base::TimeDelta kDefaultInertInterval = base::Minutes(5);

}  // namespace

BASE_FEATURE(kUserLevelMemoryPressureSignal,
             "UserLevelMemoryPressureSignal",
             base::FEATURE_DISABLED_BY_DEFAULT);

bool IsUserLevelMemoryPressureSignalEnabled() {
  return base::FeatureList::IsEnabled(kUserLevelMemoryPressureSignal);
}

base::TimeDelta MinUserMemoryPressureInterval() {
  static const base::FeatureParam<base::TimeDelta> kMinimumInterval{
      &kUserLevelMemoryPressureSignal, "minimum_interval",
      kDefaultMinimumInterval};
  return kMinimumInterval.Get();
}

base::TimeDelta InertInterval() {
  static const base::FeatureParam<base::TimeDelta> kInertInterval{
      &features::kUserLevelMemoryPressureSignal, "inert_interval_after_loading",
      kDefaultInertInterval};
  return kInertInterval.Get();
}

}  // namespace content::features
