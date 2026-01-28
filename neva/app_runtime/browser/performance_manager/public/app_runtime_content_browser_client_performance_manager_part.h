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

#ifndef NEVA_APP_RUNTIME_BROWSER_PERFORMANCE_MANAGER_PUBLIC_APP_RUNTIME_CONTENT_BROWSER_CLIENT_PERFORMANCE_MANAGER_PART_H_
#define NEVA_APP_RUNTIME_BROWSER_PERFORMANCE_MANAGER_PUBLIC_APP_RUNTIME_CONTENT_BROWSER_CLIENT_PERFORMANCE_MANAGER_PART_H_

#include "neva/app_runtime/browser/app_runtime_content_browser_client_parts.h"

namespace neva_app_runtime {

// Allows tracking RenderProcessHost lifetime and proffering the Performance
// Manager interface to new renderers.
class AppRuntimeContentBrowserClientPerformanceManagerPart
    : public AppRuntimeContentBrowserClientParts {
 public:
  AppRuntimeContentBrowserClientPerformanceManagerPart();

  AppRuntimeContentBrowserClientPerformanceManagerPart(
      const AppRuntimeContentBrowserClientPerformanceManagerPart&) = delete;
  AppRuntimeContentBrowserClientPerformanceManagerPart& operator=(
      const AppRuntimeContentBrowserClientPerformanceManagerPart&) = delete;

  ~AppRuntimeContentBrowserClientPerformanceManagerPart() override;

  // AppRuntimeContentBrowserClientParts overrides.
  void ExposeInterfacesToRenderer(
      service_manager::BinderRegistry* registry,
      blink::AssociatedInterfaceRegistry* associated_registry,
      content::RenderProcessHost* render_process_host) override;
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_BROWSER_PERFORMANCE_MANAGER_PUBLIC_APP_RUNTIME_CONTENT_BROWSER_CLIENT_PERFORMANCE_MANAGER_PART_H_
