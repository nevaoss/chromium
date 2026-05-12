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
// //chrome/browser/performance_manager/policies/memory_saver_mode_policy.h

// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NEVA_APP_RUNTIME_BROWSER_BROWSER_PERFORMANCE_MANAGER_POLICIES_MEMORY_SAVER_MODE_POLICY_H_
#define NEVA_APP_RUNTIME_BROWSER_BROWSER_PERFORMANCE_MANAGER_POLICIES_MEMORY_SAVER_MODE_POLICY_H_

#include <map>
#include <memory>

#include "base/timer/timer.h"
#include "components/performance_manager/public/graph/graph.h"
#include "components/performance_manager/public/graph/page_node.h"

namespace performance_manager::policies {

// This policy is responsible for discarding tabs after they have been
// backgrounded for a certain amount of time, when Memory Saver Mode is
// enabled.
class MemorySaverModePolicy : public GraphOwned,
                              public PageNode::ObserverDefaultImpl {
 public:
  MemorySaverModePolicy(base::TimeDelta time_before_discard);
  ~MemorySaverModePolicy() override;

  static MemorySaverModePolicy* GetInstance();

  // PageNode::ObserverDefaultImpl:
  void OnIsVisibleChanged(const PageNode* page_node) override;
  void OnBeforePageNodeRemoved(const PageNode* page_node) override;

  // GraphOwned:
  void OnPassedToGraph(Graph* graph) override;
  void OnTakenFromGraph(Graph* graph) override;

  // Returns true if Memory Saver mode is enabled, false otherwise. Useful to
  // get the state of the mode from the Performance Manager sequence.
  bool IsMemorySaverDiscardingEnabled() const;

 private:
  void StartDiscardTimerIfEnabled(const PageNode* page_node,
                                  base::TimeDelta time_before_discard);
  void RemoveActiveTimer(const PageNode* page_node);
  void DiscardPageTimerCallback(const PageNode* page_node,
                                base::LiveTicks posted_at,
                                base::TimeDelta requested_time_before_discard);

  base::TimeDelta GetTimeBeforeDiscardForCurrentMode() const;

  bool discard_background_page_enabled_ = false;
  base::TimeDelta time_before_discard_ = base::TimeDelta();

  std::map<const PageNode*, base::OneShotTimer> active_discard_timers_;
};

}  // namespace performance_manager::policies

#endif  // NEVA_APP_RUNTIME_BROWSER_BROWSER_PERFORMANCE_MANAGER_POLICIES_MEMORY_SAVER_MODE_POLICY_H_
