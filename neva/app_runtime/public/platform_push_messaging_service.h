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

#ifndef NEVA_APP_RUNTIME_PUBLIC_PLATFORM_PUSH_MESSAGING_SERVICE_H_
#define NEVA_APP_RUNTIME_PUBLIC_PLATFORM_PUSH_MESSAGING_SERVICE_H_

#include <string>

#include "neva/app_runtime/public/callback_helper.h"

namespace neva_app_runtime {

class PlatformPushMessagingService {
 public:
  struct SubscriptionInfo {
    SubscriptionInfo() = default;
    SubscriptionInfo(const std::string& service_id,
                     const std::string& x_api_key,
                     const std::string& subscription_id)
        : service_id(service_id),
          x_api_key(x_api_key),
          subscription_id(subscription_id) {}

    std::string service_id;
    std::string x_api_key;
    std::string subscription_id;
  };

  using SubscribeCallback =
      CallbackHelper<void(const std::pair<SubscriptionInfo, bool>& info)>;
  using VerifySubscriptionCallback = CallbackHelper<void(bool result)>;
  using UnsubscribeCallback = CallbackHelper<void(bool result)>;

  class Client {
   public:
    virtual void OnMessageReceived(const std::string& endpoint,
                                   const std::string& message) = 0;
    virtual ~Client() {}
  };

  virtual void SetClient(Client* client) = 0;

  virtual bool Subscribe(const std::string& app_id,
                         const std::string& authorized_entity,
                         const std::string& scope,
                         uint64_t time_to_live,
                         SubscribeCallback callback) = 0;
  virtual bool VerifySubscription(
      const std::string& app_id,
      const std::string& authorized_entity,
      const std::string& scope,
      const std::string& subscription_id,
      const std::pair<SubscriptionInfo, bool>& service_info,
      VerifySubscriptionCallback callback) = 0;
  virtual bool Unsubscribe(const std::string& app_id,
                           const std::string& authorized_entity,
                           const std::string& scope,
                           const SubscriptionInfo& service_info,
                           UnsubscribeCallback callback) = 0;
  virtual ~PlatformPushMessagingService() {}
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_PUBLIC_PLATFORM_PUSH_MESSAGING_SERVICE_H_
