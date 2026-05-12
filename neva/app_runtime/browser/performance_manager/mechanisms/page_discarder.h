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
// //chrome/browser/performance_manager/mechanisms/page_discarder.h

// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NEVA_APP_RUNTIME_BROWSER_PERFORMANCE_MANAGER_MECHANISMS_PAGE_DISCARDER_H_
#define NEVA_APP_RUNTIME_BROWSER_PERFORMANCE_MANAGER_MECHANISMS_PAGE_DISCARDER_H_

#include <vector>

#include "base/functional/callback.h"
#include "base/time/time.h"
#include "neva/app_runtime/browser/performance_manager/resource_coordinator/lifecycle_unit_state.mojom.h"

namespace performance_manager {

class PageNode;

namespace mechanism {

// Mechanism that allows discarding a PageNode.
class PageDiscarder {
 public:
  PageDiscarder() = default;
  virtual ~PageDiscarder() = default;
  PageDiscarder(const PageDiscarder& other) = delete;
  PageDiscarder& operator=(const PageDiscarder&) = delete;

  // When invoked, DiscardPageNodes() becomes a no-op.
  static void DisableForTesting();

  struct DiscardEvent {
    base::TimeTicks discard_time;
    uint64_t estimated_memory_freed_kb = 0;
  };

  // Discards |page_nodes| and runs |post_discard_cb| on the origin sequence
  // once this is done.
  virtual void DiscardPageNodes(
      const std::vector<const PageNode*>& page_nodes,
      ::mojom::LifecycleUnitDiscardReason discard_reason,
      base::OnceCallback<void(const std::vector<DiscardEvent>&)>
          post_discard_cb);
};

}  // namespace mechanism
}  // namespace performance_manager

#endif  // NEVA_APP_RUNTIME_BROWSER_PERFORMANCE_MANAGER_MECHANISMS_PAGE_DISCARDER_H_
