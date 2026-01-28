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

#ifndef NEVA_PAL_SERVICE_WEBOS_SYSTEM_SERVICEBRIDGE_DELEGATE_WEBOS_H_
#define NEVA_PAL_SERVICE_WEBOS_SYSTEM_SERVICEBRIDGE_DELEGATE_WEBOS_H_

#include "base/functional/callback.h"
#include "base/memory/weak_ptr.h"
#include "neva/pal_service/luna/luna_client.h"
#include "neva/pal_service/public/system_servicebridge_delegate.h"

#include <list>

namespace pal {
namespace webos {

class SystemServiceBridgeDelegateWebOS : public SystemServiceBridgeDelegate {
 public:
  SystemServiceBridgeDelegateWebOS(CreationParams params, Response callback);
  SystemServiceBridgeDelegateWebOS(const SystemServiceBridgeDelegateWebOS&) =
      delete;
  SystemServiceBridgeDelegateWebOS& operator=(
      const SystemServiceBridgeDelegateWebOS&) = delete;
  ~SystemServiceBridgeDelegateWebOS() override;

  void Call(std::string uri, std::string payload) override;
  void Cancel() override;

 private:
  void OnSubscription(luna::Client::ResponseStatus status,
                      unsigned token,
                      const std::string& json);
  void OnResponse(luna::Client::ResponseStatus status,
                  unsigned token,
                  const std::string& json);

  std::set<unsigned> subscription_tokens_;
  std::set<unsigned> response_tokens_;

  Response callback_;
  std::shared_ptr<luna::Client> luna_client_;
  base::WeakPtrFactory<SystemServiceBridgeDelegateWebOS> weak_factory_;
};

}  // namespace webos
}  // namespace pal

#endif  // NEVA_PAL_SERVICE_WEBOS_SYSTEM_SERVICEBRIDGE_DELEGATE_WEBOS_H_
