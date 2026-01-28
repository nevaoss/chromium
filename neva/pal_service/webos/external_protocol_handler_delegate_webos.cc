// Copyright 2023 LG Electronics, Inc.
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

#include "neva/pal_service/webos/external_protocol_handler_delegate_webos.h"

#include "base/json/json_writer.h"
#include "base/values.h"
#include "neva/pal_service/luna/luna_client.h"
#include "neva/pal_service/luna/luna_names.h"

namespace pal {
namespace webos {

const char kLaunchingMethod[] = "launch";
const char kEnactBrowserApplicationId[] = "com.webos.app.enactbrowser";

ExternalProtocolHandlerDelegateWebOS::ExternalProtocolHandlerDelegateWebOS() {
  pal::luna::Client::Params params;

  params.name = pal::luna::GetServiceNameWithPID(
      pal::luna::service_name::kApplicationManagerClient);
  luna_client_ = pal::luna::CreateClient(params);
}

ExternalProtocolHandlerDelegateWebOS::~ExternalProtocolHandlerDelegateWebOS() =
    default;

void ExternalProtocolHandlerDelegateWebOS::HandleExternalProtocol(
    const std::string& app_id,
    const std::string& translated_url) {
  LaunchEnactBrowserWithUrl(translated_url);
}

void ExternalProtocolHandlerDelegateWebOS::LaunchEnactBrowserWithUrl(
    const std::string& url) {
  base::Value::Dict value, params;
  params.Set("target", url);
  value.Set("id", kEnactBrowserApplicationId);
  value.Set("params", std::move(params));
  std::string str_param;

  if (base::JSONWriter::Write(value, &str_param)) {
    luna_client_->Call(
        pal::luna::GetServiceURI(pal::luna::service_uri::kApplicationManager,
                                 kLaunchingMethod),
        str_param);
  }
}

}  // namespace webos
}  // namespace pal
