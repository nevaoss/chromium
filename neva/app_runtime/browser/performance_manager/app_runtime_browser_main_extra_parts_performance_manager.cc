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

#include "neva/app_runtime/browser/performance_manager/public/app_runtime_browser_main_extra_parts_performance_manager.h"

#include <memory>

#include "base/command_line.h"
#include "base/feature_list.h"
#include "base/functional/bind.h"
#include "base/neva/base_switches.h"
#include "components/performance_manager/embedder/graph_features.h"
#include "components/performance_manager/embedder/performance_manager_lifetime.h"
#include "components/performance_manager/freezing/freezer.h"
#include "components/performance_manager/freezing/freezing_policy.h"
#include "components/performance_manager/graph/policies/bfcache_policy.h"
#include "components/performance_manager/graph/policies/process_priority_policy.h"
#include "components/performance_manager/performance_manager_feature_observer_client.h"
#include "components/performance_manager/public/decorators/page_load_tracker_decorator_helper.h"
#include "components/performance_manager/public/decorators/process_metrics_decorator.h"
#include "components/performance_manager/public/features.h"
#include "components/performance_manager/public/graph/graph.h"
#include "content/public/common/content_features.h"
#include "neva/app_runtime/browser/performance_manager/policies/memory_saver_mode_policy.h"
#include "neva/app_runtime/browser/performance_manager/policies/page_discarding_helper.h"

namespace {
AppRuntimeBrowserMainExtraPartsPerformanceManager* g_instance = nullptr;

// TODO(neva): Implement FreezingDiscarder once PageDiscarder is integrated into
// Neva (see NEVA-9636).
class FreezingDiscarder : public performance_manager::freezing::Discarder {
 public:
  FreezingDiscarder() = default;
  ~FreezingDiscarder() override = default;

  // performance_manager::freezing::Discarder:
  void DiscardPages(
      performance_manager::Graph* graph,
      std::vector<const performance_manager::PageNode*> page_nodes) override {
    NOTIMPLEMENTED();
  }
};
}

AppRuntimeBrowserMainExtraPartsPerformanceManager::
    AppRuntimeBrowserMainExtraPartsPerformanceManager()
    : feature_observer_client_(
          std::make_unique<
              performance_manager::PerformanceManagerFeatureObserverClient>()) {
  DCHECK(!g_instance);
  g_instance = this;
}

AppRuntimeBrowserMainExtraPartsPerformanceManager::
    ~AppRuntimeBrowserMainExtraPartsPerformanceManager() {
  DCHECK_EQ(this, g_instance);
  g_instance = nullptr;
}

// static
AppRuntimeBrowserMainExtraPartsPerformanceManager*
AppRuntimeBrowserMainExtraPartsPerformanceManager::GetInstance() {
  return g_instance;
}

// static
void AppRuntimeBrowserMainExtraPartsPerformanceManager::
    CreatePoliciesAndDecorators(performance_manager::Graph* graph) {
  graph->PassToGraph(
      std::make_unique<performance_manager::ProcessMetricsDecorator>());
  graph->PassToGraph(std::make_unique<performance_manager::FreezingPolicy>(
      std::make_unique<FreezingDiscarder>()));
  graph->PassToGraph(
      std::make_unique<performance_manager::policies::PageDiscardingHelper>());

  if (base::CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kDiscardBackgroundPageAfterSecond)) {
    int time_before_discard = 0;
    if (base::StringToInt(
            base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(
                switches::kDiscardBackgroundPageAfterSecond),
            &time_before_discard)) {
      graph->PassToGraph(std::make_unique<
                         performance_manager::policies::MemorySaverModePolicy>(
          base::Seconds(time_before_discard)));
    }
  }
}

content::FeatureObserverClient*
AppRuntimeBrowserMainExtraPartsPerformanceManager::GetFeatureObserverClient() {
  return feature_observer_client_.get();
}

void AppRuntimeBrowserMainExtraPartsPerformanceManager::PostCreateThreads() {
  performance_manager_lifetime_ =
      std::make_unique<performance_manager::PerformanceManagerLifetime>(
          performance_manager::GraphFeatures::WithMinimal(),
          base::BindOnce(&AppRuntimeBrowserMainExtraPartsPerformanceManager::
                             CreatePoliciesAndDecorators));

  page_load_tracker_decorator_helper_ =
      std::make_unique<performance_manager::PageLoadTrackerDecoratorHelper>();
}

void AppRuntimeBrowserMainExtraPartsPerformanceManager::
    PostMainMessageLoopRun() {
  page_load_tracker_decorator_helper_.reset();

  // Releasing `performance_manager_lifetime_` will tear down the registry and
  // graph safely.
  performance_manager_lifetime_.reset();
}
