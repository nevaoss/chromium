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
// //chrome/browser/performance_manager/policies/page_discarding_helper.cc

// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "neva/app_runtime/browser/performance_manager/policies/page_discarding_helper.h"

#include <memory>
#include <utility>

#include "base/byte_size.h"
#include "base/containers/flat_map.h"
#include "base/containers/flat_set.h"
#include "base/feature_list.h"
#include "base/logging.h"
#include "base/metrics/histogram_macros.h"
#include "base/sequence_checker.h"
#include "base/time/time.h"
#include "build/build_config.h"
#include "build/chromeos_buildflags.h"
#include "components/performance_manager/graph/page_node_impl.h"
#include "components/performance_manager/public/decorators/tab_page_decorator.h"
#include "components/performance_manager/public/graph/frame_node.h"
#include "components/performance_manager/public/graph/graph_operations.h"
#include "components/performance_manager/public/graph/node_attached_data.h"
#include "components/performance_manager/public/graph/node_data_describer_registry.h"
#include "components/performance_manager/public/graph/node_data_describer_util.h"
#include "components/performance_manager/public/graph/page_node.h"
#include "components/performance_manager/public/graph/process_node.h"
#include "components/performance_manager/public/user_tuning/tab_revisit_tracker.h"
#include "components/url_matcher/url_matcher.h"
#include "components/url_matcher/url_util.h"
#include "neva/app_runtime/browser/performance_manager/mechanisms/page_discarder.h"
#include "third_party/abseil-cpp/absl/cleanup/cleanup.h"
#include "url/gurl.h"

using performance_manager::mechanism::PageDiscarder;

namespace performance_manager {
namespace policies {
namespace {

// NodeAttachedData used to indicate that there's already been an attempt to
// discard a PageNode.
// TODO(sebmarchand): The only reason for a discard attempt to fail is if we try
// to discard a prerenderer, remove this once we can detect if a PageNode is a
// prerenderer in CanDiscard().
class DiscardAttemptMarker : public NodeAttachedDataImpl<DiscardAttemptMarker> {
 public:
  explicit DiscardAttemptMarker(const PageNodeImpl* page_node) {}
  ~DiscardAttemptMarker() override = default;
};

const char kDescriberName[] = "PageDiscardingHelper";

using NodeRssMap = base::flat_map<const PageNode*, base::ByteSize>;

// Returns the mapping from page_node to its RSS estimation.
NodeRssMap GetPageNodeRssEstimate(
    const std::vector<PageNodeSortProxy>& candidates) {
  // Initialize the result map in one shot for time complexity O(n * log(n)).
  NodeRssMap::container_type result_container;
  result_container.reserve(candidates.size());
  for (auto candidate : candidates) {
    result_container.emplace_back(candidate.page_node(), base::ByteSize(0));
  }
  NodeRssMap result(std::move(result_container));

  // TODO(crbug.com/40194476): Use visitor to accumulate the result to avoid
  // allocating extra lists of frame nodes behind the scenes.

  // List all the processes associated with these page nodes.
  base::flat_set<const ProcessNode*> process_nodes;
  for (auto candidate : candidates) {
    base::flat_set<const ProcessNode*> processes =
        GraphOperations::GetAssociatedProcessNodes(candidate.page_node());
    process_nodes.insert(processes.begin(), processes.end());
  }

  // Compute the resident set of each page by simply summing up the estimated
  // resident set of all its frames.
  for (const ProcessNode* process_node : process_nodes) {
    ProcessNode::NodeSetView<const FrameNode*> process_frames =
        process_node->GetFrameNodes();
    if (!process_frames.size()) {
      continue;
    }
    // Get the resident set of the process and split it equally across its
    // frames.
    const base::ByteSize frame_rss =
        process_node->GetResidentSet() / process_frames.size();
    for (const FrameNode* frame_node : process_frames) {
      // Check if the frame belongs to a discardable page, if so update the
      // resident set of the page.
      auto iter = result.find(frame_node->GetPageNode());
      if (iter == result.end()) {
        continue;
      }
      iter->second += frame_rss;
    }
  }
  return result;
}

void RecordDiscardedTabMetrics(const PageNodeSortProxy& candidate) {
  // Logs a histogram entry to track the proportion of discarded tabs that
  // were protected at the time of discard.
  UMA_HISTOGRAM_BOOLEAN("Discarding.DiscardingProtectedTab",
                        candidate.is_protected());

  // Logs a histogram entry to track the proportion of discarded tabs that
  // were focused at the time of discard.
  UMA_HISTOGRAM_BOOLEAN("Discarding.DiscardingFocusedTab",
                        candidate.is_focused());
}

}  // namespace

PageDiscardingHelper::PageDiscardingHelper()
    : page_discarder_(std::make_unique<PageDiscarder>()) {}
PageDiscardingHelper::~PageDiscardingHelper() = default;

void PageDiscardingHelper::DiscardAPage(
    base::OnceCallback<void(bool)> post_discard_cb,
    DiscardReason discard_reason,
    base::TimeDelta minimum_time_in_background) {
  DiscardMultiplePages(std::nullopt, false, std::move(post_discard_cb),
                       discard_reason, minimum_time_in_background);
}

void PageDiscardingHelper::DiscardMultiplePages(
    std::optional<memory_pressure::ReclaimTarget> reclaim_target,
    bool discard_protected_tabs,
    base::OnceCallback<void(bool)> post_discard_cb,
    DiscardReason discard_reason,
    base::TimeDelta minimum_time_in_background) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  LOG(WARNING) << "Discarding multiple pages with target (kb): "
               << (reclaim_target ? reclaim_target->target.InKiB() : 0);

  if (reclaim_target) {
    unnecessary_discard_monitor_.OnReclaimTargetBegin(*reclaim_target);
  }

  // Ensures running post_discard_cb on early return.
  absl::Cleanup run_post_discard_cb_on_return = [&post_discard_cb] {
    std::move(post_discard_cb).Run(false);
  };

  std::vector<PageNodeSortProxy> candidates;
  for (const PageNode* page_node : GetOwningGraph()->GetAllPageNodes()) {
    CanDiscardResult can_discard_result =
        CanDiscard(page_node, discard_reason, minimum_time_in_background);
    if (can_discard_result == CanDiscardResult::kMarked) {
      continue;
    }
    bool is_protected = (can_discard_result == CanDiscardResult::kProtected);
    if (!discard_protected_tabs && is_protected) {
      continue;
    }
    candidates.emplace_back(page_node, false, page_node->IsVisible(),
                            is_protected, page_node->IsFocused(),
                            page_node->GetLastVisibilityChangeTime());
  }

  // Sorts with ascending importance.
  std::sort(candidates.begin(), candidates.end());

  UMA_HISTOGRAM_COUNTS_100("Discarding.DiscardCandidatesCount",
                           candidates.size());

  // Returns early when candidate is empty to avoid infinite loop in
  // DiscardMultiplePages and PostDiscardAttemptCallback.
  if (candidates.empty()) {
    return;
  }
  std::vector<const PageNode*> discard_attempts;

  if (!reclaim_target) {
    const PageNode* oldest = candidates[0].page_node();
    discard_attempts.emplace_back(oldest);

    // Record metrics about the tab that is about to be discarded.
    RecordDiscardedTabMetrics(candidates[0]);
  } else {
    const base::ByteSize reclaim_target_value = reclaim_target->target;
    base::ByteSize total_reclaim;
    NodeRssMap page_node_rss = GetPageNodeRssEstimate(candidates);
    for (auto& candidate : candidates) {
      if (total_reclaim >= reclaim_target_value) {
        break;
      }
      const PageNode* node = candidate.page_node();
      discard_attempts.emplace_back(node);

      // Record metrics about the tab that is about to be discarded.
      RecordDiscardedTabMetrics(candidate);

      // The node RSS value is updated by ProcessMetricsDecorator periodically.
      // The RSS value is 0 for nodes that have never been updated, estimate the
      // RSS value to 80 MiB for these nodes. 80 MiB is the average
      // Memory.Renderer.PrivateMemoryFootprint histogram value on Windows in
      // August 2021.
      std::optional<base::ByteSize> node_reclaim =
          page_node_rss[node].is_zero() ? base::MiBU(80) : page_node_rss[node];
      total_reclaim += node_reclaim.value();

      LOG(WARNING) << "Queueing discard attempt, type="
                   << performance_manager::PageNode::ToString(node->GetType())
                   << ", flags=[" << (candidate.is_focused() ? " focused" : "")
                   << (candidate.is_protected() ? " protected" : "")
                   << (candidate.is_visible() ? " visible" : "")
                   << " ] to save " << node_reclaim.value();
    }
  }

  if (discard_attempts.empty()) {
    return;
  }

  // Adorns the PageNodes with a discard attempt marker to make sure that we
  // don't try to discard it multiple times if it fails to be discarded. In
  // practice this should only happen to prerenderers.
  for (auto* attempt : discard_attempts) {
    DiscardAttemptMarker::GetOrCreate(PageNodeImpl::FromNode(attempt));
  }

  // Got to the end successfully, don't call the early return callback.
  std::move(run_post_discard_cb_on_return).Cancel();

  page_discarder_->DiscardPageNodes(
      discard_attempts, discard_reason,
      base::BindOnce(&PageDiscardingHelper::PostDiscardAttemptCallback,
                     weak_factory_.GetWeakPtr(), reclaim_target,
                     discard_protected_tabs, std::move(post_discard_cb),
                     discard_reason, minimum_time_in_background));
}

void PageDiscardingHelper::ImmediatelyDiscardMultiplePages(
    const std::vector<const PageNode*>& page_nodes,
    DiscardReason discard_reason,
    base::OnceCallback<void(bool)> post_discard_cb) {
  std::vector<const PageNode*> eligible_nodes;
  for (const PageNode* node : page_nodes) {
    // Pass 0 TimeDelta to bypass the minimum time in background check.
    if (CanDiscard(node, discard_reason,
                   /*minimum_time_in_background=*/base::TimeDelta()) ==
        CanDiscardResult::kEligible) {
      eligible_nodes.emplace_back(node);
    }
  }

  if (eligible_nodes.empty()) {
    std::move(post_discard_cb).Run(false);
  } else {
    page_discarder_->DiscardPageNodes(
        std::move(eligible_nodes), discard_reason,
        base::BindOnce(
            [](base::OnceCallback<void(bool)> callback,
               const std::vector<PageDiscarder::DiscardEvent>& discard_events) {
              std::move(callback).Run(discard_events.size() > 0);
            },
            std::move(post_discard_cb)));
  }
}

void PageDiscardingHelper::OnPassedToGraph(Graph* graph) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  graph->GetNodeDataDescriberRegistry()->RegisterDescriber(this,
                                                           kDescriberName);
}

void PageDiscardingHelper::OnTakenFromGraph(Graph* graph) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  graph->GetNodeDataDescriberRegistry()->UnregisterDescriber(this);
}

const PageLiveStateDecorator::Data*
PageDiscardingHelper::GetPageNodeLiveStateData(
    const PageNode* page_node) const {
  return PageLiveStateDecorator::Data::FromPageNode(page_node);
}

PageDiscardingHelper::CanDiscardResult PageDiscardingHelper::CanDiscard(
    const PageNode* page_node,
    DiscardReason discard_reason,
    base::TimeDelta minimum_time_in_background) const {
  if (DiscardAttemptMarker::Get(PageNodeImpl::FromNode(page_node))) {
    return CanDiscardResult::kMarked;
  }

  bool is_proactive_or_suggested;
  switch (discard_reason) {
    case DiscardReason::EXTERNAL:
      // Always allow discards from external sources like extensions.
      return CanDiscardResult::kEligible;
    case DiscardReason::URGENT:
      is_proactive_or_suggested = false;
      break;
    case DiscardReason::PROACTIVE:
      is_proactive_or_suggested = true;
      break;
    case DiscardReason::SUGGESTED:
      is_proactive_or_suggested = true;
      break;
  }

  if (page_node->IsVisible()) {
    return CanDiscardResult::kProtected;
  }
  // Don't discard tabs that are playing or have recently played audio.
  if (page_node->IsAudible()) {
    return CanDiscardResult::kProtected;
  } else if (page_node->GetTimeSinceLastAudibleChange().value_or(
                 base::TimeDelta::Max()) < kTabAudioProtectionTime) {
    return CanDiscardResult::kProtected;
  }

  if (base::TimeTicks::Now() - page_node->GetLastVisibilityChangeTime() <
      minimum_time_in_background) {
    return CanDiscardResult::kProtected;
  }

  // Don't discard pages that are displaying content in picture-in-picture.
  if (page_node->HasPictureInPicture()) {
    return CanDiscardResult::kProtected;
  }

  // Do not discard PDFs as they might contain entry that is not saved and they
  // don't remember their scrolling positions. See crbug.com/547286 and
  // crbug.com/65244.
  if (page_node->GetContentsMimeType() == "application/pdf") {
    return CanDiscardResult::kProtected;
  }

  // Don't discard tabs that don't have a main frame yet.
  // TODO(crbug.com/40910297): Due to a state tracking bug, sometimes there are
  // two frames marked "current". In that case GetPrimaryMainFrameNode() returns an
  // arbitrary one, which may not have the url set correctly. As a workaround
  // ignore the returned frame and use GetMainFrameUrl() for the url.
  if (!page_node->GetPrimaryMainFrameNode()) {
    return CanDiscardResult::kProtected;
  }

  // Only discard http(s) pages and internal pages to make sure that we don't
  // discard extensions or other PageNode that don't correspond to a tab.
  const GURL& main_frame_url = page_node->GetMainFrameUrl();
  bool is_web_page_or_internal_page_or_file =
      main_frame_url.SchemeIsHTTPOrHTTPS() ||
      main_frame_url.SchemeIs("chrome") || main_frame_url.SchemeIsFile();
  if (!is_web_page_or_internal_page_or_file) {
    return CanDiscardResult::kProtected;
  }

  if (!main_frame_url.is_valid() || main_frame_url.is_empty()) {
    return CanDiscardResult::kProtected;
  }

  // The enterprise policy to except pages from discarding applies to both
  // proactive and urgent discards.
  if (IsPageOptedOutOfDiscarding(page_node->GetBrowserContextID(),
                                 main_frame_url)) {
    return CanDiscardResult::kProtected;
  }

  if (is_proactive_or_suggested &&
      page_node->GetNotificationPermissionStatus() ==
          blink::mojom::PermissionStatus::GRANTED) {
    return CanDiscardResult::kProtected;
  }

  const auto* live_state_data = GetPageNodeLiveStateData(page_node);

  // The live state data won't be available if none of these events ever
  // happened on the page.
  if (live_state_data) {
    // Don't discard the page if an extension is protecting it from discards.
    if (!live_state_data->IsAutoDiscardable()) {
      return CanDiscardResult::kProtected;
    }
    if (live_state_data->IsCapturingVideo()) {
      return CanDiscardResult::kProtected;
    }
    if (live_state_data->IsCapturingAudio()) {
      return CanDiscardResult::kProtected;
    }
    if (live_state_data->IsBeingMirrored()) {
      return CanDiscardResult::kProtected;
    }
    if (live_state_data->IsCapturingWindow()) {
      return CanDiscardResult::kProtected;
    }
    if (live_state_data->IsCapturingDisplay()) {
      return CanDiscardResult::kProtected;
    }
    if (live_state_data->IsConnectedToBluetoothDevice()) {
      return CanDiscardResult::kProtected;
    }
    if (live_state_data->IsConnectedToUSBDevice()) {
      return CanDiscardResult::kProtected;
    }
    // Don't discard the active tab in any window, even if the window is not
    // visible. Otherwise the user would see a blank page when the window
    // becomes visible again, as the tab isn't reloaded until they click on it.
    if (live_state_data->IsActiveTab()) {
      return CanDiscardResult::kProtected;
    }
    // Pinning a tab is a strong signal the user wants to keep it.
    if (live_state_data->IsPinnedTab()) {
      return CanDiscardResult::kProtected;
    }
    // Don't discard pages with devtools attached, because when it's restored
    // the devtools window won't come back. The user may be monitoring the page
    // in the background with devtools.
    if (live_state_data->IsDevToolsOpen()) {
      return CanDiscardResult::kProtected;
    }
    if (is_proactive_or_suggested &&
        live_state_data->UpdatedTitleOrFaviconInBackground()) {
      return CanDiscardResult::kProtected;
    }
  }

  // `HadUserEdits()` is currently a superset of `HadFormInteraction()` but
  // that may change so check both here (the check is not expensive).
  if (page_node->HadFormInteraction() || page_node->HadUserEdits()) {
    return CanDiscardResult::kProtected;
  }

  // TODO(sebmarchand): Do not discard crashed tabs.

  return CanDiscardResult::kEligible;
}

bool PageDiscardingHelper::IsPageOptedOutOfDiscarding(
    const base::UnguessableToken& browser_context_id,
    const GURL& url) const {
  auto it = profiles_no_discard_patterns_.find(browser_context_id);
  if (it == profiles_no_discard_patterns_.end()) {
    return false;
  }
  return !it->second->MatchURL(url).empty();
}

void PageDiscardingHelper::PostDiscardAttemptCallback(
    std::optional<memory_pressure::ReclaimTarget> reclaim_target,
    bool discard_protected_tabs,
    base::OnceCallback<void(bool)> post_discard_cb,
    DiscardReason discard_reason,
    base::TimeDelta minimum_time_in_background,
    const std::vector<PageDiscarder::DiscardEvent>& discard_events) {
  // When there is no discard candidate, DiscardMultiplePages returns
  // early and PostDiscardAttemptCallback is not called.
  if (discard_events.empty()) {
    // DiscardAttemptMarker will force the retry to choose different pages.
    DiscardMultiplePages(reclaim_target, discard_protected_tabs,
                         std::move(post_discard_cb), discard_reason,
                         minimum_time_in_background);
    return;
  }

  for (const auto& discard_event : discard_events) {
    unnecessary_discard_monitor_.OnDiscard(
        discard_event.estimated_memory_freed,
        discard_event.discard_time);
  }

  unnecessary_discard_monitor_.OnReclaimTargetEnd();

  std::move(post_discard_cb).Run(true);
}

}  // namespace policies
}  // namespace performance_manager
