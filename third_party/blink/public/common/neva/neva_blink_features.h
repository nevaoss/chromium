// Copyright 2024 LG Electronics, Inc.
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

#ifndef THIRD_PARTY_BLINK_PUBLIC_COMMON_NEVA_NEVA_BLINK_FEATURES_H_
#define THIRD_PARTY_BLINK_PUBLIC_COMMON_NEVA_NEVA_BLINK_FEATURES_H_

#include "base/feature_list.h"
#include "base/metrics/field_trial_params.h"
#include "base/time/time.h"
#include "third_party/blink/public/common/common_export.h"

namespace blink {
namespace features {
namespace neva {

BLINK_COMMON_EXPORT BASE_DECLARE_FEATURE(kSetServiceWorkerIdleDelay);
BLINK_COMMON_EXPORT extern const base::FeatureParam<base::TimeDelta>
    kServiceWorkerDefaultIdleDelay;

}  // namespace neva
}  // namespace features
}  // namespace blink

#endif  // THIRD_PARTY_BLINK_PUBLIC_COMMON_NEVA_NEVA_BLINK_FEATURES_H_
