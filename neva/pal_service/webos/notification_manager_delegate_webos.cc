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

#include "neva/pal_service/webos/notification_manager_delegate_webos.h"

#include "base/json/json_writer.h"
#include "base/memory/singleton.h"
#include "base/values.h"
#include "neva/pal_service/luna/luna_client.h"
#include "neva/pal_service/luna/luna_names.h"

namespace {
const char kSourceId[] = "sourceId";
const char kCreateToast[] = "createToast";
const char kMessage[] = "message";

class LunaClient final {
 public:
  static LunaClient* Get();
  LunaClient(const LunaClient&) = delete;
  LunaClient& operator=(const LunaClient&) = delete;
  void CreateToast(const std::string& app_id, const std::string& msg);

 private:
  friend struct base::DefaultSingletonTraits<LunaClient>;

  LunaClient();
  ~LunaClient() = default;
  std::unique_ptr<pal::luna::Client> luna_client_;
};

// static
LunaClient* LunaClient::Get() {
  return base::Singleton<LunaClient>::get();
}

LunaClient::LunaClient() {
  pal::luna::Client::Params params;
  params.name = pal::luna::GetServiceNameWithPID(
      pal::luna::service_name::kNotificationClient);
  luna_client_ = pal::luna::CreateClient(params);
}

void LunaClient::CreateToast(const std::string& app_id,
                             const std::string& msg) {
  base::Value::Dict value;
  value.Set(kSourceId, app_id);
  value.Set(kMessage, msg);
  std::string param;

  if (base::JSONWriter::Write(value, &param)) {
    luna_client_->Call(pal::luna::GetServiceURI(
                           pal::luna::service_uri::kNotification, kCreateToast),
                       param);
  }
}
}  // namespace

namespace pal {
namespace webos {

NotificationManagerDelegateWebOS::NotificationManagerDelegateWebOS() = default;
NotificationManagerDelegateWebOS::~NotificationManagerDelegateWebOS() = default;

void NotificationManagerDelegateWebOS::CreateToast(const std::string& app_id,
                                                   const std::string& msg) {
  LunaClient::Get()->CreateToast(app_id, msg);
}

}  // namespace webos
}  // namespace pal
