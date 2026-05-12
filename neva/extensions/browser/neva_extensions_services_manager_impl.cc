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

#include "neva/extensions/browser/neva_extensions_services_manager_impl.h"

#include "base/memory/singleton.h"
#include "content/public/browser/browser_context.h"
#include "neva/app_runtime/browser/app_runtime_browser_context.h"
#include "neva/app_runtime/browser/app_runtime_storage_partition_name.h"
#include "neva/extensions/browser/neva_extensions_service_impl.h"
#include "neva/logging.h"

namespace neva {

NevaExtensionsServicesManagerImpl::NevaExtensionsServicesManagerImpl()
    : services_map_() {}

NevaExtensionsServicesManagerImpl::~NevaExtensionsServicesManagerImpl() =
    default;

// static
void NevaExtensionsServicesManagerImpl::BindForRenderer(
    int render_process_id,
    mojo::PendingReceiver<mojom::NevaExtensionsServicesManager> receiver) {
  NevaExtensionsServicesManagerImpl::GetInstance()->BindReceiver(
      std::move(receiver));
}

// static
NevaExtensionsServicesManagerImpl*
NevaExtensionsServicesManagerImpl::GetInstance() {
  return base::Singleton<NevaExtensionsServicesManagerImpl>::get();
}

void NevaExtensionsServicesManagerImpl::BindReceiver(
    mojo::PendingReceiver<mojom::NevaExtensionsServicesManager> receiver) {
  receivers_.Add(this, std::move(receiver));
}

void NevaExtensionsServicesManagerImpl::RegisterClient(
    mojo::PendingAssociatedRemote<mojom::NevaExtensionsServicesManagerClient>
        remote) {
  remotes_.Add(std::move(remote));
}

void NevaExtensionsServicesManagerImpl::BindNewExtensionsServiceByContext(
    mojo::PendingReceiver<mojom::NevaExtensionsService> receiver,
    const std::string& partition) {
  neva_app_runtime::StoragePartitionInfo partition_info =
      neva_app_runtime::ParseStoragePartitionName(partition);
  content::BrowserContext* context =
      neva_app_runtime::AppRuntimeBrowserContext::From(
          partition_info.name, partition_info.off_the_record);
  auto service = services_map_.find(context);

  if (service != services_map_.end()) {
    service->second->Bind(std::move(receiver));
  }
}

NevaExtensionsServiceImpl*
NevaExtensionsServicesManagerImpl::GetNevaExtensionsServiceFor(
    content::BrowserContext* context) {
  if (services_map_.find(context) != services_map_.end()) {
    return services_map_.find(context)->second;
  } else {
    auto item =
        services_map_.insert({context, new NevaExtensionsServiceImpl(context)});

    for (auto& remote : remotes_) {
      std::string partition =
          static_cast<neva_app_runtime::AppRuntimeBrowserContext*>(context)
              ->GetName();

      if (!context->IsOffTheRecord()) {
        partition.insert(0, "persist:");
      }

      LOG(INFO) << __func__ << " Registered extensions service for partition "
                << partition;
      remote->OnExtensionServiceRegistered(partition);
    }

    return item.first->second;
  }
}

}  // namespace neva
