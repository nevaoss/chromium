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

#include "neva/pal_service/network_error_page_controller.h"

#include "neva/pal_service/pal_platform_factory.h"
#include "neva/pal_service/public/network_error_page_controller_delegate.h"

namespace pal {

NetworkErrorPageControllerImpl::NetworkErrorPageControllerImpl()
    : delegate_(
          PlatformFactory::Get()->CreateNetworkErrorPageControllerDelegate()) {}

NetworkErrorPageControllerImpl::~NetworkErrorPageControllerImpl() {}

void NetworkErrorPageControllerImpl::AddBinding(
    mojo::PendingReceiver<mojom::NetworkErrorPageController> receiver) {
  receivers_.Add(this, std::move(receiver));
}

void NetworkErrorPageControllerImpl::LaunchNetworkSettings(int target_id) {
  if (delegate_)
    delegate_->LaunchNetworkSettings(target_id);
}

void NetworkErrorPageControllerImpl::CloseShellIfNeeded() {
  if (delegate_)
    delegate_->CloseShellIfNeeded();
}

}  // namespace pal
