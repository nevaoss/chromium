// Copyright 2022 LG Electronics, Inc.
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

#include "neva/extensions/browser/neva_extension_api_client.h"

#include "extensions/browser/api/messaging/messaging_delegate.h"
#include "neva/extensions/browser/api/management/neva_management_api_delegate.h"
#include "neva/extensions/browser/api/messaging/neva_messaging_delegate.h"

namespace neva {

NevaExtensionsAPIClient::NevaExtensionsAPIClient() {}

NevaExtensionsAPIClient::~NevaExtensionsAPIClient() {}

extensions::MessagingDelegate* NevaExtensionsAPIClient::GetMessagingDelegate() {
  if (!messaging_delegate_)
    messaging_delegate_ = std::make_unique<NevaMessagingDelegate>();
  return messaging_delegate_.get();
}

extensions::ManagementAPIDelegate*
NevaExtensionsAPIClient::CreateManagementAPIDelegate() const {
  return new NevaManagementAPIDelegate;
}

}  // namespace neva
