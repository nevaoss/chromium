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

// Based on
// //chorme/browser/resource_coordinator/tab_lifecycle_unit_external.h

// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NEVA_APP_RUNTIME_BROWSER_PERFORMANCE_MANAGER_RESOURCE_COORDINATOR_TAB_LIFECYCLE_UNIT_EXTERNAL_H_
#define NEVA_APP_RUNTIME_BROWSER_PERFORMANCE_MANAGER_RESOURCE_COORDINATOR_TAB_LIFECYCLE_UNIT_EXTERNAL_H_

#include "neva/app_runtime/browser/performance_manager/resource_coordinator/lifecycle_unit_state.mojom.h"

namespace content {
class WebContents;
}  // namespace content

namespace resource_coordinator {

class TabLifecycleObserver;

// Interface to control the lifecycle of a tab exposed outside of
// chrome/browser/resource_coordinator/.
class TabLifecycleUnitExternal {
 public:
  virtual ~TabLifecycleUnitExternal() = default;

  // Returns the WebContents associated with this tab.
  virtual content::WebContents* GetWebContents() const = 0;

  // Discards the tab.
  virtual bool DiscardTab(mojom::LifecycleUnitDiscardReason reason,
                          uint64_t memory_footprint_estimate = 0) = 0;
};

}  // namespace resource_coordinator

#endif  // NEVA_APP_RUNTIME_BROWSER_PERFORMANCE_MANAGER_RESOURCE_COORDINATOR_TAB_LIFECYCLE_UNIT_EXTERNAL_H_
