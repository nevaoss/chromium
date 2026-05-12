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

#ifndef NEVA_APP_RUNTIME_BROWSER_PERFORMANCE_MANAGER_PUBLIC_CHROME_BROWSER_MAIN_EXTRA_PARTS_PERFORMANCE_MANAGER_H_
#define NEVA_APP_RUNTIME_BROWSER_PERFORMANCE_MANAGER_PUBLIC_CHROME_BROWSER_MAIN_EXTRA_PARTS_PERFORMANCE_MANAGER_H_

#include <memory>

#include "neva/app_runtime/browser/app_runtime_browser_main_extra_parts.h"

namespace content {
class FeatureObserverClient;
}

namespace performance_manager {
class Graph;
class PageLoadTrackerDecoratorHelper;
class PerformanceManagerFeatureObserverClient;
class PerformanceManagerLifetime;
}  // namespace performance_manager

// Handles the initialization of the performance manager and a few dependent
// classes that create/manage graph nodes.
class AppRuntimeBrowserMainExtraPartsPerformanceManager
    : public neva_app_runtime::AppRuntimeBrowserMainExtraParts {
 public:
  AppRuntimeBrowserMainExtraPartsPerformanceManager();

  AppRuntimeBrowserMainExtraPartsPerformanceManager(
      const AppRuntimeBrowserMainExtraPartsPerformanceManager&) = delete;
  AppRuntimeBrowserMainExtraPartsPerformanceManager& operator=(
      const AppRuntimeBrowserMainExtraPartsPerformanceManager&) = delete;

  ~AppRuntimeBrowserMainExtraPartsPerformanceManager() override;

  // Returns the only instance of this class.
  static AppRuntimeBrowserMainExtraPartsPerformanceManager* GetInstance();

  // Returns the FeatureObserverClient that should be exposed to //content to
  // allow the performance manager to track usage of features in frames. Valid
  // to call from any thread, but external synchronization is needed to make
  // sure that the performance manager is available.
  content::FeatureObserverClient* GetFeatureObserverClient();

 private:
  static void CreatePoliciesAndDecorators(performance_manager::Graph* graph);

  // AppRuntimeBrowserMainExtraParts overrides.
  void PostCreateThreads() override;
  void PostMainMessageLoopRun() override;

  // Manages the lifetime of the PerformanceManager graph and registry for the
  // browser process.
  std::unique_ptr<performance_manager::PerformanceManagerLifetime>
      performance_manager_lifetime_;

  const std::unique_ptr<
      performance_manager::PerformanceManagerFeatureObserverClient>
      feature_observer_client_;

  // Needed to maintain the PageNode::IsLoading() property.
  std::unique_ptr<performance_manager::PageLoadTrackerDecoratorHelper>
      page_load_tracker_decorator_helper_;
};

#endif  // NEVA_APP_RUNTIME_BROWSER_PERFORMANCE_MANAGER_PUBLIC_CHROME_BROWSER_MAIN_EXTRA_PARTS_PERFORMANCE_MANAGER_H_
