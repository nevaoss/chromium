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

#ifndef NEVA_PAL_SERVICE_WEBOS_MEMORYMANAGER_DELEGATE_WEBOS_H_
#define NEVA_PAL_SERVICE_WEBOS_MEMORYMANAGER_DELEGATE_WEBOS_H_

#include "base/functional/callback.h"
#include "base/logging.h"
#include "neva/pal_service/luna/luna_client.h"
#include "neva/pal_service/public/memorymanager_delegate.h"

#include <memory>
#include <string>

namespace pal {
namespace webos {
namespace {
const char kGetMemoryStatusRequest[] = "{}";
const char kSignalAddMatch[] = "signal/addmatch";
const char kDefaultResponse[] = "{}";
}  // namespace

class MemoryManagerDelegateWebOS : public MemoryManagerDelegate {
 public:
  MemoryManagerDelegateWebOS();
  MemoryManagerDelegateWebOS(const MemoryManagerDelegateWebOS&) = delete;
  MemoryManagerDelegateWebOS& operator=(const MemoryManagerDelegateWebOS&) =
      delete;
  ~MemoryManagerDelegateWebOS() override;

  static std::unique_ptr<MemoryManagerDelegateWebOS> Create();

  void GetMemoryStatus(OnceResponse callback) override = 0;
  void SubscribeToLevelChanged(RepeatingResponse callback) override = 0;
  void UnsubscribeFromLevelChanged() override;
  bool IsSubscribed() const override;
  std::string ConvertToMemoryLevel(const std::string& level);

 protected:
  std::unique_ptr<luna::Client> luna_client_;
  bool subscribed_ = false;
  unsigned subscription_token_ = 0;
  RepeatingResponse subscription_callback_;
};

}  // namespace webos
}  // namespace pal

#endif  // NEVA_PAL_SERVICE_WEBOS_MEMORYMANAGER_DELEGATE_WEBOS_H_
