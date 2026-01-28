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

#ifndef NEVA_BROWSER_SHELL_SERVICE_BROWSER_SHELL_PERMISSIONS_IMPL_H_
#define NEVA_BROWSER_SHELL_SERVICE_BROWSER_SHELL_PERMISSIONS_IMPL_H_

#include <map>

#include "base/memory/weak_ptr.h"
#include "mojo/public/cpp/bindings/associated_remote.h"
#include "mojo/public/cpp/bindings/pending_associated_remote.h"
#include "mojo/public/cpp/bindings/receiver_set.h"
#include "mojo/public/cpp/bindings/remote_set.h"
#include "neva/app_runtime/browser/permissions/app_runtime_permission_prompt_factory.h"
#include "neva/browser_shell/service/public/mojom/browser_shell_permissions.mojom.h"

namespace browser_shell {

class ShellServiceImpl;

class PermissionsImpl :
    public mojom::Permissions,
    public neva_app_runtime::AppRuntimePermissionPromptFactory {
 public:
  PermissionsImpl(ShellServiceImpl* shell_service);
  PermissionsImpl(const PermissionsImpl&) = delete;
  PermissionsImpl& operator=(const PermissionsImpl&) = delete;
  ~PermissionsImpl() override;

  void AddBinding(mojo::PendingReceiver<mojom::Permissions> receiver);

  // mojom::Permissions:
  void RegisterClient(
      mojo::PendingAssociatedRemote<mojom::PermissionsClient> remote) override;
  void DecidePermission(mojom::PermissionResult result,
                        uint64_t id,
                        uint64_t page_id) override;

  // neva_app_runtime::AppRuntimePermissionPromptFactory:
  std::unique_ptr<permissions::PermissionPrompt> CreatePermissionPrompt(
      content::WebContents* web_contents,
      permissions::PermissionPrompt::Delegate* delegate) override;

 private:
  void Initialize();

  const raw_ptr<ShellServiceImpl> shell_service_;
  uint64_t permission_prompt_id_ = 0;
  std::map<uint64_t, base::WeakPtr<permissions::PermissionPrompt::Delegate>>
      permission_prompt_delegates_;

  mojo::ReceiverSet<mojom::Permissions> receivers_;
  mojo::AssociatedRemoteSet<mojom::PermissionsClient> remotes_;
  base::WeakPtrFactory<PermissionsImpl> weak_ptr_factory_{this};
};

}  // namespace browser_shell

#endif  // NEVA_BROWSER_SHELL_SERVICE_BROWSER_SHELL_PERMISSIONS_IMPL_H_
