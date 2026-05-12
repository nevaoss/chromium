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
// //chrome/browser/resource_coordinator/tab_lifecycle_unit_source.cc

// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "neva/app_runtime/browser/performance_manager/resource_coordinator/tab_lifecycle_unit_source.h"

#include <utility>

#include "base/check_op.h"
#include "base/functional/bind.h"
#include "base/memory/weak_ptr.h"
#include "base/observer_list.h"
#include "components/performance_manager/public/graph/graph.h"
#include "components/performance_manager/public/graph/page_node.h"
#include "components/performance_manager/public/performance_manager.h"
#include "components/prefs/pref_change_registrar.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_user_data.h"
#include "neva/app_runtime/browser/performance_manager/resource_coordinator/tab_lifecycle_unit.h"

namespace resource_coordinator {

// Allows storage of a TabLifecycleUnit on a WebContents.
class TabLifecycleUnitSource::TabLifecycleUnitHolder
    : public content::WebContentsUserData<
          TabLifecycleUnitSource::TabLifecycleUnitHolder> {
 public:
  TabLifecycleUnitHolder(const TabLifecycleUnitHolder&) = delete;
  TabLifecycleUnitHolder& operator=(const TabLifecycleUnitHolder&) = delete;

  ~TabLifecycleUnitHolder() override = default;

  TabLifecycleUnit* lifecycle_unit() const { return lifecycle_unit_.get(); }
  void set_lifecycle_unit(std::unique_ptr<TabLifecycleUnit> lifecycle_unit) {
    lifecycle_unit_ = std::move(lifecycle_unit);
  }
  std::unique_ptr<TabLifecycleUnit> TakeTabLifecycleUnit() {
    return std::move(lifecycle_unit_);
  }

 private:
  friend class content::WebContentsUserData<TabLifecycleUnitHolder>;

  explicit TabLifecycleUnitHolder(content::WebContents* web_contents)
      : content::WebContentsUserData<
            TabLifecycleUnitSource::TabLifecycleUnitHolder>(*web_contents) {}

  std::unique_ptr<TabLifecycleUnit> lifecycle_unit_;
  WEB_CONTENTS_USER_DATA_KEY_DECL();
};

WEB_CONTENTS_USER_DATA_KEY_IMPL(TabLifecycleUnitSource::TabLifecycleUnitHolder);

// A very simple graph observer that forwards events over to the
// TabLifecycleUnitSource on the UI thread. This is created on the UI thread
// and ownership passed to the performance manager.
class TabLifecycleStateObserver
    : public performance_manager::PageNode::ObserverDefaultImpl,
      public performance_manager::GraphOwned {
 public:
  using Graph = performance_manager::Graph;
  using PageNode = performance_manager::PageNode;

  TabLifecycleStateObserver() = default;

  TabLifecycleStateObserver(const TabLifecycleStateObserver&) = delete;
  TabLifecycleStateObserver& operator=(const TabLifecycleStateObserver&) =
      delete;

  ~TabLifecycleStateObserver() override = default;

 private:
  static void OnLifecycleStateChangedImpl(
      base::WeakPtr<content::WebContents> contents,
      performance_manager::mojom::LifecycleState state) {
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    // If the web contents is still alive then dispatch to the actual
    // implementation in TabLifecycleUnitSource.
    if (contents) {
      TabLifecycleUnitSource::OnLifecycleStateChanged(contents.get(), state);
    }
  }

  // PageNode::ObserverDefaultImpl:
  void OnPageLifecycleStateChanged(const PageNode* page_node) override {
    // Forward the notification over to the UI thread.
    content::GetUIThreadTaskRunner({})->PostTask(
        FROM_HERE,
        base::BindOnce(&TabLifecycleStateObserver::OnLifecycleStateChangedImpl,
                       page_node->GetWebContents(),
                       page_node->GetLifecycleState()));
  }

  void OnPassedToGraph(Graph* graph) override {
    graph->AddPageNodeObserver(this);
  }

  void OnTakenFromGraph(Graph* graph) override {
    graph->RemovePageNodeObserver(this);
  }
};

TabLifecycleUnitSource::TabLifecycleUnitSource() = default;

TabLifecycleUnitSource::~TabLifecycleUnitSource() = default;

void TabLifecycleUnitSource::Start() {
  // TODO(sebmarchand): Remove the "IsAvailable" check, or merge the TM into the
  // PM. The TM and PM must always exist together.
  if (performance_manager::PerformanceManager::IsAvailable()) {
    performance_manager::PerformanceManager::PassToGraph(
        FROM_HERE, std::make_unique<TabLifecycleStateObserver>());
  }
}

// static
TabLifecycleUnitExternal* TabLifecycleUnitSource::GetTabLifecycleUnitExternal(
    content::WebContents* web_contents) {
  auto* lu = GetTabLifecycleUnit(web_contents);
  if (!lu) {
    return nullptr;
  }
  return lu->AsTabLifecycleUnitExternal();
}

// static
TabLifecycleUnitSource::TabLifecycleUnit*
TabLifecycleUnitSource::GetTabLifecycleUnit(
    content::WebContents* web_contents) {
  auto* holder = TabLifecycleUnitHolder::FromWebContents(web_contents);
  if (!holder) {
    TabLifecycleUnitHolder::CreateForWebContents(web_contents);
    holder = TabLifecycleUnitHolder::FromWebContents(web_contents);
    holder->set_lifecycle_unit(
        std::make_unique<TabLifecycleUnit>(web_contents));
  }
  return holder->lifecycle_unit();
}

// static
void TabLifecycleUnitSource::OnLifecycleStateChanged(
    content::WebContents* web_contents,
    performance_manager::mojom::LifecycleState state) {
  TabLifecycleUnit* lifecycle_unit = GetTabLifecycleUnit(web_contents);

  // Lifecycle state is updated independently from navigations. Therefore, there
  // is no need to filter out the event if it was generated before the last
  // navigation.
  if (lifecycle_unit) {
    lifecycle_unit->UpdateLifecycleState(state);
  }
}

}  // namespace resource_coordinator
