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

// Based on //chrome/browser/resource_coordinator/tab_lifecycle_unit.h

// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NEVA_APP_RUNTIME_BROWSER_PERFORMANCE_MANAGER_RESOURCE_COORDINATOR_TAB_LIFECYCLE_UNIT_H_
#define NEVA_APP_RUNTIME_BROWSER_PERFORMANCE_MANAGER_RESOURCE_COORDINATOR_TAB_LIFECYCLE_UNIT_H_

#include "base/memory/raw_ptr.h"
#include "base/observer_list.h"
#include "base/time/time.h"
#include "components/performance_manager/public/mojom/coordination_unit.mojom-forward.h"
#include "content/public/browser/visibility.h"
#include "content/public/browser/web_contents_observer.h"
#include "neva/app_runtime/browser/performance_manager/resource_coordinator/tab_lifecycle_unit_external.h"
#include "neva/app_runtime/browser/performance_manager/resource_coordinator/tab_lifecycle_unit_source.h"

namespace content {
class RenderProcessHost;
class WebContents;
}  // namespace content

namespace resource_coordinator {

using ::mojom::LifecycleUnitDiscardReason;
using ::mojom::LifecycleUnitState;
using ::mojom::LifecycleUnitStateChangeReason;

class TabLifecycleUnitExternalImpl;

// Represents a tab.
class TabLifecycleUnitSource::TabLifecycleUnit
    : public content::WebContentsObserver {
 public:
  TabLifecycleUnit(content::WebContents* web_contents);

  TabLifecycleUnit(const TabLifecycleUnit&) = delete;
  TabLifecycleUnit& operator=(const TabLifecycleUnit&) = delete;

  ~TabLifecycleUnit() override;

  // Updates the tab's lifecycle state when changed outside the tab
  // lifecycle unit.
  void UpdateLifecycleState(performance_manager::mojom::LifecycleState state);

  // LifecycleUnit:
  TabLifecycleUnitExternal* AsTabLifecycleUnitExternal();

  bool Discard(LifecycleUnitDiscardReason discard_reason,
               uint64_t memory_footprint_estimate);

  LifecycleUnitState GetState() const;

 protected:
  // Sets the state of this LifecycleUnit to |state| and notifies observers.
  // |reason| indicates what caused the state change.
  void SetState(LifecycleUnitState state,
                LifecycleUnitStateChangeReason reason);

 private:
  // Finishes a tab discard, invoked by Discard().
  void FinishDiscard(LifecycleUnitDiscardReason discard_reason,
                     uint64_t tab_resident_set_size_estimate);

  // Returns the RenderProcessHost associated with this tab.
  content::RenderProcessHost* GetRenderProcessHost() const;

  // Maintains the most recent LifecycleUnitDiscardReason that was passed into
  // Discard().
  LifecycleUnitDiscardReason discard_reason_ =
      LifecycleUnitDiscardReason::EXTERNAL;

  // Current state of this LifecycleUnit.
  LifecycleUnitState state_ = LifecycleUnitState::ACTIVE;

  std::unique_ptr<TabLifecycleUnitExternalImpl> external_impl_;
};

}  // namespace resource_coordinator

#endif  // NEVA_APP_RUNTIME_BROWSER_PERFORMANCE_MANAGER_RESOURCE_COORDINATOR_TAB_LIFECYCLE_UNIT_H_
