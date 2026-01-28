// Copyright 2018 LG Electronics, Inc.
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

#include "base/neva/base_switches.h"

namespace switches {

// V8 snapshot blob path
const char kV8SnapshotBlobPath[] = "v8-snapshot-blob-path";

// Layer tree setting for decoded image working set budget in MB
const char kDecodedImageWorkingSetBudgetMB[] =
    "decoded-image-working-set-budget-mb";

// Custom discardable memory limit in MB used by discardable memory component
const char kDiscardableMemoryLimitMB[] = "discardable-memory-limit-mb";

// Discard the background page after the given time in seconds.
// When the value is 0, it means the page will be discard immediately when
// changed to background state. If the switch itself is not given then it means
// background page will not be discarded.
const char kDiscardBackgroundPageAfterSecond[] =
    "discard-background-page-after-second";

// Supports external protocols(ex: mailto) handling
const char kEnableExternalProtocolsHandling[] =
    "enable-external-protocols-handling";

// When using unsupported feature in app_runtime (e.g. File download, upload),
// notification occurs.
const char kEnableNotificationForUnsupportedFeatures[] =
    "enable-notification-for-unsupported-features";

// The factor by which to reduce the GPU memory size of the cache when under
// memory pressure.
const char kMemPressureGPUCacheSizeReductionFactor[] =
    "mem-pressure-gpu-cache-size-reduction-factor";

// The factor by which to reduce the tile manager low memory policy bytes
// limit when under memory pressure.
const char kTileManagerLowMemPolicyBytesLimitReductionFactor[] =
    "tile-manager-low-mem-policy-bytes-limit-reduction-factor";

// Limits size of local storage for the second level domain in MB
const char kLocalStorageLimitPerSecondLevelDomain[] =
    "local-storage-limit-per-second-level-domain";

}  // namespace switches
