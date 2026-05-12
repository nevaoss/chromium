// Copyright 2019 LG Electronics, Inc.
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

#include "neva/pal_service/memorymanager.h"

#include "base/memory/weak_ptr.h"
#include "neva/pal_service/pal_platform_factory.h"
#include "neva/pal_service/public/memorymanager_delegate.h"

namespace pal {

MemoryManagerImpl::MemoryManagerImpl()
  : delegate_(PlatformFactory::Get()->CreateMemoryManagerDelegate()),
    weak_factory_(this) {}

MemoryManagerImpl::~MemoryManagerImpl() {}

void MemoryManagerImpl::AddBinding(
    mojo::PendingReceiver<mojom::MemoryManager> receiver) {
  receivers_.Add(this, std::move(receiver));
}

void MemoryManagerImpl::GetMemoryStatus(GetMemoryStatusCallback callback) {
  if (delegate_)
    delegate_->GetMemoryStatus(std::move(callback));
}

void MemoryManagerImpl::Subscribe(SubscribeCallback callback) {
  if (!delegate_)
    return;

  if (listeners_.empty()) {
    delegate_->SubscribeToLevelChanged(base::BindRepeating(
        &MemoryManagerImpl::OnLevelChanged, weak_factory_.GetWeakPtr()));
  }

  mojo::AssociatedRemote<mojom::MemoryManagerListener> listener;
  std::move(callback).Run(listener.BindNewEndpointAndPassReceiver());
  listeners_.Add(std::move(listener));
}

void MemoryManagerImpl::OnLevelChanged(const std::string& current) {
  if (listeners_.empty()) {
    delegate_->UnsubscribeFromLevelChanged();
  } else {
    for (auto& listener : listeners_)
      listener->LevelChanged(current);
  }
}

}  // namespace pal
