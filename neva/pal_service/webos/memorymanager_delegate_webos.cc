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

#include "neva/pal_service/webos/memorymanager_delegate_webos.h"

#include "base/json/json_reader.h"
#include "neva/pal_service/luna/luna_names.h"

namespace pal {
namespace webos {
MemoryManagerDelegateWebOS::MemoryManagerDelegateWebOS() {
  using namespace luna;
  Client::Params params;
  params.name =
      luna::GetServiceNameWithRandSuffix(service_name::kChromiumMemory);
  luna_client_ = CreateClient(params);
}

MemoryManagerDelegateWebOS::~MemoryManagerDelegateWebOS() = default;

void MemoryManagerDelegateWebOS::UnsubscribeFromLevelChanged() {
  if (subscribed_) {
    luna_client_->Unsubscribe(subscription_token_);
    subscribed_ = false;
  }
}

bool MemoryManagerDelegateWebOS::IsSubscribed() const {
  return subscribed_;
}

std::string MemoryManagerDelegateWebOS::ConvertToMemoryLevel(
    const std::string& level) {
  if ((level == "critical") || (level == "reboot"))
    return kMemoryLevelCritical;
  else if (level == "low")
    return kMemoryLevelLow;
  else if ((level == "medium") || (level == "normal"))
    return kMemoryLevelNormal;
  LOG(ERROR) << __func__ << " unknown memory level: " << level;
  return "";
}

}  // namespace webos
}  // namespace pal
