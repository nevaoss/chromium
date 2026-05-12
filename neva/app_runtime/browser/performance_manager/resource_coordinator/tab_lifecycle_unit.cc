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

// Based on //chrome/browser/resource_coordinator/tab_lifecycle_unit.cc

// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "neva/app_runtime/browser/performance_manager/resource_coordinator/tab_lifecycle_unit.h"

#include <memory>
#include <optional>
#include <utility>

#include "base/feature_list.h"
#include "base/functional/bind.h"
#include "base/logging.h"
#include "base/memory/raw_ptr.h"
#include "base/observer_list.h"
#include "base/process/process_metrics.h"
#include "build/chromeos_buildflags.h"
#include "components/device_event_log/device_event_log.h"
#include "components/performance_manager/public/decorators/page_live_state_decorator.h"
#include "components/permissions/permission_manager.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/web_contents.h"
#include "neva/app_runtime/browser/performance_manager/webview_map.h"
#include "third_party/blink/public/mojom/frame/sudden_termination_disabler_type.mojom.h"
#include "url/gurl.h"

namespace resource_coordinator {

namespace {

using StateChangeReason = LifecycleUnitStateChangeReason;

// Returns true if it is valid to transition from |from| to |to| for |reason|.
bool IsValidStateChange(LifecycleUnitState from,
                        LifecycleUnitState to,
                        StateChangeReason reason) {
  switch (from) {
    case LifecycleUnitState::ACTIVE: {
      switch (to) {
        // Discard(URGENT|EXTERNAL) is called.
        case LifecycleUnitState::DISCARDED: {
          return reason == StateChangeReason::BROWSER_INITIATED ||
                 reason == StateChangeReason::SYSTEM_MEMORY_PRESSURE ||
                 reason == StateChangeReason::EXTENSION_INITIATED;
        }
        case LifecycleUnitState::FROZEN: {
          // Render-initiated freezing, which happens when freezing a page
          // through ChromeDriver.
          return reason == StateChangeReason::RENDERER_INITIATED;
        }
        default:
          return false;
      }
    }
    case LifecycleUnitState::THROTTLED: {
      return false;
    }
    case LifecycleUnitState::FROZEN: {
      switch (to) {
        // The renderer notifies the browser that the page was unfrozen after
        // it became visible.
        case LifecycleUnitState::ACTIVE: {
          return reason == StateChangeReason::RENDERER_INITIATED;
        }
        // Discard(URGENT|EXTERNAL) is called.
        case LifecycleUnitState::DISCARDED: {
          return reason == StateChangeReason::BROWSER_INITIATED ||
                 reason == StateChangeReason::SYSTEM_MEMORY_PRESSURE ||
                 reason == StateChangeReason::EXTENSION_INITIATED;
        }
        default:
          return false;
      }
    }
    case LifecycleUnitState::DISCARDED: {
      switch (to) {
        // The WebContents is focused or reloaded.
        case LifecycleUnitState::ACTIVE:
          return reason == StateChangeReason::USER_INITIATED;
        default:
          return false;
      }
    }
  }
}

StateChangeReason DiscardReasonToStateChangeReason(
    LifecycleUnitDiscardReason reason) {
  switch (reason) {
    case LifecycleUnitDiscardReason::EXTERNAL:
      return StateChangeReason::EXTENSION_INITIATED;
    case LifecycleUnitDiscardReason::URGENT:
      return StateChangeReason::SYSTEM_MEMORY_PRESSURE;
    case LifecycleUnitDiscardReason::PROACTIVE:
      return StateChangeReason::BROWSER_INITIATED;
    case LifecycleUnitDiscardReason::SUGGESTED:
      return StateChangeReason::BROWSER_INITIATED;
  }
}

}  // namespace

class TabLifecycleUnitExternalImpl : public TabLifecycleUnitExternal {
 public:
  explicit TabLifecycleUnitExternalImpl(
      TabLifecycleUnitSource::TabLifecycleUnit* tab_lifecycle_unit)
      : tab_lifecycle_unit_(tab_lifecycle_unit) {}

  // TabLifecycleUnitExternal:

  content::WebContents* GetWebContents() const override {
    return tab_lifecycle_unit_->web_contents();
  }

  bool DiscardTab(LifecycleUnitDiscardReason reason,
                  uint64_t memory_footprint_estimate) override {
    return tab_lifecycle_unit_->Discard(reason, memory_footprint_estimate);
  }

 private:
  raw_ptr<TabLifecycleUnitSource::TabLifecycleUnit> tab_lifecycle_unit_ =
      nullptr;
};

TabLifecycleUnitSource::TabLifecycleUnit::TabLifecycleUnit(
    content::WebContents* web_contents)
    : content::WebContentsObserver(web_contents) {}

TabLifecycleUnitSource::TabLifecycleUnit::~TabLifecycleUnit() {}

void TabLifecycleUnitSource::TabLifecycleUnit::UpdateLifecycleState(
    performance_manager::mojom::LifecycleState state) {
  switch (state) {
    case performance_manager::mojom::LifecycleState::kFrozen: {
      SetState(LifecycleUnitState::FROZEN,
               StateChangeReason::RENDERER_INITIATED);
      break;
    }

    case performance_manager::mojom::LifecycleState::kRunning: {
      SetState(LifecycleUnitState::ACTIVE,
               StateChangeReason::RENDERER_INITIATED);
      break;
    }

    default: {
      NOTREACHED_IN_MIGRATION();
      break;
    }
  }
}

TabLifecycleUnitExternal*
TabLifecycleUnitSource::TabLifecycleUnit::AsTabLifecycleUnitExternal() {
  // Create an impl the first time this is called.
  if (!external_impl_) {
    external_impl_ = std::make_unique<TabLifecycleUnitExternalImpl>(this);
  }
  return external_impl_.get();
}

void TabLifecycleUnitSource::TabLifecycleUnit::FinishDiscard(
    LifecycleUnitDiscardReason discard_reason,
    uint64_t tab_memory_footprint_estimate) {
  LOG(INFO) << "Discarding \"" << web_contents()->GetTitle() << "\" page.";
  content::WebContents* const old_contents = web_contents();
  content::WebContents::CreateParams create_params(
      web_contents()->GetBrowserContext());
  // TODO(fdoray): Consider setting |initially_hidden| to true when the tab is
  // OCCLUDED. Will require checking that the tab reload correctly when it
  // becomes VISIBLE.
  create_params.initially_hidden =
      old_contents->GetVisibility() == content::Visibility::HIDDEN;
  create_params.desired_renderer_state =
      content::WebContents::CreateParams::kNoRendererProcess;
  create_params.last_active_time = old_contents->GetLastActiveTime();
  std::unique_ptr<content::WebContents> null_contents =
      content::WebContents::Create(create_params);

  // Send the notification to WebContentsObservers that the old content is about
  // to be discarded and replaced with `null_contents`.
  old_contents->AboutToBeDiscarded(null_contents.get());

  // Copy over the state from the navigation controller to preserve the
  // back/forward history and to continue to display the correct title/favicon.
  //
  // Set |needs_reload| to false so that the tab is not automatically reloaded
  // when activated. If it was true, there would be an immediate reload when the
  // active tab of a non-visible window is discarded. SetFocused() will take
  // care of reloading the tab when it becomes active in a focused window.
  null_contents->GetController().CopyStateFrom(&old_contents->GetController(),
                                               /* needs_reload */ false);

  // First try to fast-kill the process, if it's just running a single tab.
  GetRenderProcessHost()->FastShutdownIfPossible(1u, true);

  // This ensures that on reload after discard, the document has
  // "WasDiscarded" set to true.
  // The "WasDiscarded" state is also sent to tab_strip_model.
  null_contents->SetWasDiscarded(true);

  neva_app_runtime::WebView* webview =
      neva_app_runtime::WebViewMap::GetInstance()->FindWebViewFromWebContents(
          old_contents);

  if (!webview) {
    LOG(ERROR) << "Cannot find WebView for WebContents " << old_contents;
    return;
  }

  std::unique_ptr<content::WebContents> old_contents_deleter =
      webview->DiscardWebContents(std::move(null_contents));
  neva_app_runtime::WebViewMap::GetInstance()->RemoveWebContents(old_contents);

  // Discard the old tab's renderer.
  // TODO(jamescook): This breaks script connections with other tabs. Find a
  // different approach that doesn't do that, perhaps based on
  // RenderFrameProxyHosts.
  old_contents_deleter.reset();

  SetState(LifecycleUnitState::DISCARDED,
           DiscardReasonToStateChangeReason(discard_reason));
}

bool TabLifecycleUnitSource::TabLifecycleUnit::Discard(
    LifecycleUnitDiscardReason reason,
    uint64_t tab_memory_footprint_estimate) {
  if (!IsValidStateChange(GetState(), LifecycleUnitState::DISCARDED,
                          DiscardReasonToStateChangeReason(reason))) {
    // Logs are used to diagnose user feedback reports.
    MEMORY_LOG(ERROR) << "Skipped discarding unit because a transition from "
                      << GetState() << " to discarded is not allowed.";
    return false;
  }

  discard_reason_ = reason;

  FinishDiscard(reason, tab_memory_footprint_estimate);

  return true;
}

LifecycleUnitState TabLifecycleUnitSource::TabLifecycleUnit::GetState() const {
  return state_;
}

void TabLifecycleUnitSource::TabLifecycleUnit::SetState(
    LifecycleUnitState state,
    LifecycleUnitStateChangeReason reason) {
  state_ = state;
}

content::RenderProcessHost*
TabLifecycleUnitSource::TabLifecycleUnit::GetRenderProcessHost() const {
  return web_contents()->GetPrimaryMainFrame()->GetProcess();
}

}  // namespace resource_coordinator
