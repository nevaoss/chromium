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
// //chrome/browser/performance_manager/policies/memory_saver_mode_policy.cc

// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "neva/app_runtime/browser/performance_manager/policies/memory_saver_mode_policy.h"

#include "base/containers/contains.h"
#include "base/strings/string_number_conversions.h"
#include "components/performance_manager/public/features.h"
#include "neva/app_runtime/browser/performance_manager/policies/page_discarding_helper.h"
#include "neva/app_runtime/browser/performance_manager/webview_map.h"

namespace performance_manager::policies {

namespace {
MemorySaverModePolicy* g_memory_saver_mode_policy = nullptr;
}  // namespace

MemorySaverModePolicy::MemorySaverModePolicy(
    base::TimeDelta time_before_discard) {
  DCHECK(!g_memory_saver_mode_policy);
  g_memory_saver_mode_policy = this;

  time_before_discard_ = time_before_discard;
}

MemorySaverModePolicy::~MemorySaverModePolicy() {
  DCHECK_EQ(this, g_memory_saver_mode_policy);
  g_memory_saver_mode_policy = nullptr;
}

// static
MemorySaverModePolicy* MemorySaverModePolicy::GetInstance() {
  return g_memory_saver_mode_policy;
}

void MemorySaverModePolicy::OnIsVisibleChanged(const PageNode* page_node) {
  if (page_node->GetLoadingState() ==
          PageNode::LoadingState::kLoadingNotStarted ||
      !neva_app_runtime::WebViewMap::GetInstance()->FindWebViewFromWebContents(
          page_node->GetWebContents().get())) {
    return;
  }

  // If the page is made visible, any existing timers that refer to it should be
  // cancelled. `RemoveActiveTimer` handles the case where no timer exists
  // gracefully.
  if (page_node->IsVisible()) {
    RemoveActiveTimer(page_node);
  } else {
    StartDiscardTimerIfEnabled(page_node, GetTimeBeforeDiscardForCurrentMode());
  }
}

void MemorySaverModePolicy::OnBeforePageNodeRemoved(const PageNode* page_node) {
  RemoveActiveTimer(page_node);
}

void MemorySaverModePolicy::OnPassedToGraph(Graph* graph) {
  graph->AddPageNodeObserver(this);
}

void MemorySaverModePolicy::OnTakenFromGraph(Graph* graph) {
  // The logic in this class depends on being notified of pages being removed,
  // otherwise there's no guarantee PageNode pointers are still valid when
  // timers fire. To avoid possibly having callbacks manipulate invalid PageNode
  // pointers, clear all the existing timers before unregistering the observer.
  active_discard_timers_.clear();

  graph->RemovePageNodeObserver(this);
}

bool MemorySaverModePolicy::IsMemorySaverDiscardingEnabled() const {
  return true;
}

void MemorySaverModePolicy::StartDiscardTimerIfEnabled(
    const PageNode* page_node,
    base::TimeDelta time_before_discard) {
  if (IsMemorySaverDiscardingEnabled()) {
    // Memory Saver mode is enabled, so the tab should be discarded after the
    // amount of time specified by finch is elapsed.
    CHECK_NE(time_before_discard, base::TimeDelta::Max());
    active_discard_timers_[page_node].Start(
        FROM_HERE, time_before_discard,
        base::BindOnce(&MemorySaverModePolicy::DiscardPageTimerCallback,
                       base::Unretained(this), page_node,
                       base::LiveTicks::Now(), time_before_discard));
  }
}

void MemorySaverModePolicy::RemoveActiveTimer(const PageNode* page_node) {
  // If there's a discard timer already running for this page, erase it from the
  // map which will stop the timer when it is destroyed.
  active_discard_timers_.erase(page_node);
}

void MemorySaverModePolicy::DiscardPageTimerCallback(
    const PageNode* page_node,
    base::LiveTicks posted_at,
    base::TimeDelta requested_time_before_discard) {
  RemoveActiveTimer(page_node);

  // Turning off Memory Saver Mode would delete the timer, so it's not
  // possible to get here and for Memory Saver Mode to be off.
  DCHECK(IsMemorySaverDiscardingEnabled());

  // If the time elapsed according to `LiveTicks` is shorter than
  // `requested_time_before_discard`, it means that the device was in a
  // suspended state at some point between when the timer was started and now.
  // In this case, start a new timer for the difference, which is the remaining
  // time the tab should stay backgrounded to total
  // `requested_time_before_discard` in background.
  base::TimeDelta elapsed_not_suspended = base::LiveTicks::Now() - posted_at;
  if (elapsed_not_suspended < requested_time_before_discard) {
    StartDiscardTimerIfEnabled(
        page_node, requested_time_before_discard - elapsed_not_suspended);
  } else {
    GetOwningGraph()
        ->GetRegisteredObjectAs<PageDiscardingHelper>()
        ->ImmediatelyDiscardMultiplePages(
            {page_node}, PageDiscardingHelper::DiscardReason::PROACTIVE);
  }
}

base::TimeDelta MemorySaverModePolicy::GetTimeBeforeDiscardForCurrentMode()
    const {
  return time_before_discard_;
}

}  // namespace performance_manager::policies
