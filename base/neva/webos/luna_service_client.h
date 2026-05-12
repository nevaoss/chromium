// Copyright 2014-2019 LG Electronics, Inc.
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

#ifndef BASE_NEVA_WEBOS_LUNA_SERVICE_CLIENT_H_
#define BASE_NEVA_WEBOS_LUNA_SERVICE_CLIENT_H_

#include <luna-service2/lunaservice.h>

#include <map>
#include <string>

#include "base/base_export.h"
#include "base/functional/callback.h"

namespace base {

class LunaServiceClient {
 public:
  enum URIType {
    AUDIO = 0,
    SETTING,
    MEDIACONTROLLER,
    URITypeMax = MEDIACONTROLLER,
  };

  enum class ClientType { MEDIA };

  using ResponseCB = base::RepeatingCallback<void(const std::string&)>;

  struct ResponseHandlerWrapper {
    LunaServiceClient::ResponseCB callback;
    std::string uri;
    std::string param;
  };

  static std::string GetServiceURI(URIType type, const std::string& action);

  explicit LunaServiceClient(ClientType type);
  explicit LunaServiceClient(const std::string& identifier,
                             bool application_service = false);
  LunaServiceClient(const LunaServiceClient&) = delete;
  LunaServiceClient& operator=(const LunaServiceClient&) = delete;
  ~LunaServiceClient();

  bool CallAsync(const std::string& uri, const std::string& param);
  bool CallAsync(const std::string& uri,
                 const std::string& param,
                 const ResponseCB& callback);
  bool Subscribe(const std::string& uri,
                 const std::string& param,
                 LSMessageToken* subscribeKey,
                 const ResponseCB& callback);
  bool Unsubscribe(LSMessageToken subscribeKey);

 private:
  void Initialize(const std::string& identifier, bool application_service);
  bool RegisterService(const std::string& name);
  bool RegisterApplicationService(const std::string& appid);
  bool UnregisterService();

  LSHandle* handle_ = nullptr;
  GMainContext* context_ = nullptr;
  std::map<LSMessageToken, std::unique_ptr<ResponseHandlerWrapper>> handlers_;
};

}  // namespace base

#endif  // BASE_NEVA_WEBOS_LUNA_SERVICE_CLIENT_H_
