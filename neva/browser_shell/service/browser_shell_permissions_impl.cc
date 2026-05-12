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

#include "neva/browser_shell/service/browser_shell_permissions_impl.h"

#include "components/permissions/request_type.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "neva/app_runtime/app/app_runtime_page_contents.h"
#include "neva/app_runtime/browser/permissions/app_runtime_permission_prompt.h"
#include "neva/app_runtime/browser/permissions/app_runtime_permissions_client.h"
#include "neva/logging.h"

namespace browser_shell {

PermissionsImpl::PermissionsImpl(ShellServiceImpl* shell_service)
    : shell_service_(shell_service) {
  Initialize();
}

PermissionsImpl::~PermissionsImpl() = default;

void PermissionsImpl::AddBinding(
    mojo::PendingReceiver<mojom::Permissions> receiver) {
  receivers_.Add(this, std::move(receiver));
}

void PermissionsImpl::RegisterClient(
    mojo::PendingAssociatedRemote<mojom::PermissionsClient> remote) {
  remotes_.Add(std::move(remote));
}

void PermissionsImpl::DecidePermission(mojom::PermissionResult result,
                                       uint64_t id,
                                       uint64_t page_id) {
  auto delegate_it = permission_prompt_delegates_.find(id);
  if (delegate_it == permission_prompt_delegates_.end()) {
    LOG(WARNING) << __func__
                 << " No matching permission prompt found. Do nothing.";
    return;
  }

  base::WeakPtr<permissions::PermissionPrompt::Delegate> delegate =
      std::move(delegate_it->second);
  permission_prompt_delegates_.erase(delegate_it);
  if (!delegate) {
    return;
  }

  switch (result){
    case mojom::PermissionResult::ACCEPTED:
      delegate->Accept();
      break;

    case mojom::PermissionResult::ACCEPTED_THIS_TIME:
      delegate->AcceptThisTime();
      break;

    case mojom::PermissionResult::DENIED:
      delegate->Deny();
      break;

    case mojom::PermissionResult::DISMISSED:
      delegate->Dismiss();
      break;

    case mojom::PermissionResult::IGNORED:
      delegate->Ignore();
      break;

    default:
      LOG(WARNING) << __func__
                   << " Unknown permission decision. Ignore applied.";
      delegate->Ignore();
      break;
  }
}

std::unique_ptr<permissions::PermissionPrompt>
PermissionsImpl::CreatePermissionPrompt(
    content::WebContents* web_contents,
    permissions::PermissionPrompt::Delegate* delegate) {
  uint64_t id = ++permission_prompt_id_;
  if (delegate) {
    permission_prompt_delegates_.insert({id, delegate->GetWeakPtr()});

    std::string requesting_origin = delegate->GetRequestingOrigin().spec();
    std::vector<std::string> permission_keys;
    auto& requests = delegate->Requests();
    for (auto request : requests) {
      const char* permission_key_ptr =
          permissions::PermissionKeyForRequestType(request->request_type());
      if (permission_key_ptr) {
        permission_keys.push_back(std::string(permission_key_ptr));
      }
    }

    auto* page_contents = neva_app_runtime::PageContents::From(web_contents);
    const uint64_t page_id = page_contents ? page_contents->GetID() : 0;
    for (auto& remote : remotes_) {
      remote->OnPermissionRequested(requesting_origin, permission_keys, id,
                                    page_id);
    }

    return std::make_unique<neva_app_runtime::AppRuntimePermissionPrompt>(
        web_contents, delegate);
  }
  return nullptr;
}

void PermissionsImpl::Initialize() {
  if (content::BrowserThread::CurrentlyOn(content::BrowserThread::UI)) {
    auto* permission_client_ =
        neva_app_runtime::AppRuntimePermissionsClient::GetInstance();
    if (permission_client_) {
       permission_client_->SetPromptFactory(weak_ptr_factory_.GetWeakPtr());
    }
    return;
  }

  LOG(WARNING) << __func__
               << " Permissions initialization should be called on UI Thread.";
  content::GetUIThreadTaskRunner({})->PostTask(
      FROM_HERE, base::BindOnce(&PermissionsImpl::Initialize,
                                weak_ptr_factory_.GetWeakPtr()));
}

}  // namespace browser_shell
