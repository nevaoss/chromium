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

#ifndef NEVA_EXTENSIONS_BROWSER_NEVA_EXTENSIONS_SERVICES_MANAGER_IMPL_H_
#define NEVA_EXTENSIONS_BROWSER_NEVA_EXTENSIONS_SERVICES_MANAGER_IMPL_H_

#include <map>
#include <memory>

#include "base/memory/singleton.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "mojo/public/cpp/bindings/receiver_set.h"
#include "mojo/public/cpp/bindings/remote_set.h"
#include "neva/extensions/common/mojom/neva_extensions_services_manager.mojom.h"

namespace content {
class BrowserContext;
}

namespace neva {

class NevaExtensionsServiceImpl;

class NevaExtensionsServicesManagerImpl
    : public mojom::NevaExtensionsServicesManager {
 public:
  // static
  static void BindForRenderer(
      int render_process_id,
      mojo::PendingReceiver<mojom::NevaExtensionsServicesManager> receiver);
  static NevaExtensionsServicesManagerImpl* GetInstance();

  NevaExtensionsServicesManagerImpl(const NevaExtensionsServicesManagerImpl&) =
      delete;
  NevaExtensionsServicesManagerImpl& operator=(
      const NevaExtensionsServicesManagerImpl&) = delete;

  NevaExtensionsServiceImpl* GetNevaExtensionsServiceFor(
      content::BrowserContext* context);

  void BindReceiver(
      mojo::PendingReceiver<mojom::NevaExtensionsServicesManager> receiver);

  // mojom
  void RegisterClient(
      mojo::PendingAssociatedRemote<mojom::NevaExtensionsServicesManagerClient>)
      override;
  void BindNewExtensionsServiceByContext(
      mojo::PendingReceiver<mojom::NevaExtensionsService> receiver,
      const std::string& partition) override;

 private:
  friend struct base::DefaultSingletonTraits<NevaExtensionsServicesManagerImpl>;

  NevaExtensionsServicesManagerImpl();
  ~NevaExtensionsServicesManagerImpl() override;

  std::map<content::BrowserContext*, NevaExtensionsServiceImpl*> services_map_;

  mojo::ReceiverSet<mojom::NevaExtensionsServicesManager> receivers_;
  mojo::AssociatedRemoteSet<mojom::NevaExtensionsServicesManagerClient>
      remotes_;
};

}  // namespace neva

#endif  // NEVA_EXTENSIONS_BROWSER_NEVA_EXTENSIONS_SERVICES_MANAGER_IMPL_H_
