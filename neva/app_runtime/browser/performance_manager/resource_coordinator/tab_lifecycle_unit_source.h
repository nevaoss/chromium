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
// //chrome/browser/resource_coordinator/tab_lifecycle_unit_source.h

// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NEVA_APP_RUNTIME_BROWSER_PERFORMANCE_MANAGER_RESOURCE_COORDINATOR_TAB_LIFECYCLE_UNIT_SOURCE_H_
#define NEVA_APP_RUNTIME_BROWSER_PERFORMANCE_MANAGER_RESOURCE_COORDINATOR_TAB_LIFECYCLE_UNIT_SOURCE_H_

#include "base/memory/raw_ptr.h"
#include "base/observer_list.h"
#include "components/performance_manager/public/mojom/coordination_unit.mojom.h"
#include "components/performance_manager/public/mojom/lifecycle.mojom-forward.h"

namespace content {
class WebContents;
}

namespace resource_coordinator {

class TabLifecycleStateObserver;
class TabLifecycleUnitExternal;

// Creates and destroys LifecycleUnits as tabs are created and destroyed.
class TabLifecycleUnitSource {
 public:
  class TabLifecycleUnit;
  class LifecycleStateObserver;

  explicit TabLifecycleUnitSource();

  TabLifecycleUnitSource(const TabLifecycleUnitSource&) = delete;
  TabLifecycleUnitSource& operator=(const TabLifecycleUnitSource&) = delete;

  ~TabLifecycleUnitSource();

  // Should be called once all the dependencies of this class have been created
  // (e.g. the global PerformanceManager instance).
  void Start();

  // Returns the TabLifecycleUnitExternal instance associated with
  // |web_contents|, or nullptr if |web_contents| isn't a tab.
  static TabLifecycleUnitExternal* GetTabLifecycleUnitExternal(
      content::WebContents* web_contents);

 protected:
  class TabLifecycleUnitHolder;

 private:
  friend class TabLifecycleStateObserver;

  // Returns the TabLifecycleUnit instance associated with |web_contents|, or
  // nullptr if |web_contents| isn't a tab.
  static TabLifecycleUnit* GetTabLifecycleUnit(
      content::WebContents* web_contents);

  // This is called indirectly from the corresponding event on a PageNode in the
  // performance_manager Graph.
  static void OnLifecycleStateChanged(
      content::WebContents* web_contents,
      performance_manager::mojom::LifecycleState state);
};

}  // namespace resource_coordinator

#endif  // NEVA_APP_RUNTIME_BROWSER_PERFORMANCE_MANAGER_RESOURCE_COORDINATOR_TAB_LIFECYCLE_UNIT_SOURCE_H_
