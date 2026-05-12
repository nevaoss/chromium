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

#include "neva/app_runtime/browser/performance_manager/public/app_runtime_content_browser_client_performance_manager_part.h"

#include "components/performance_manager/embedder/performance_manager_registry.h"

namespace neva_app_runtime {

AppRuntimeContentBrowserClientPerformanceManagerPart::
    AppRuntimeContentBrowserClientPerformanceManagerPart() = default;
AppRuntimeContentBrowserClientPerformanceManagerPart::
    ~AppRuntimeContentBrowserClientPerformanceManagerPart() = default;

void AppRuntimeContentBrowserClientPerformanceManagerPart::
    ExposeInterfacesToRenderer(
        service_manager::BinderRegistry* registry,
        blink::AssociatedInterfaceRegistry* associated_registry_unusued,
        content::RenderProcessHost* render_process_host) {
  auto* performance_manager_registry =
      performance_manager::PerformanceManagerRegistry::GetInstance();
  if (performance_manager_registry) {
    performance_manager_registry->CreateProcessNode(render_process_host);
  }
}

}  // namespace neva_app_runtime
