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

#include "neva/pal_service/webos/application_registrator_delegate_webos.h"

#include "base/command_line.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/logging.h"
#include "neva/pal_service/luna/luna_names.h"
#include "neva/pal_service/webos/switches.h"

namespace {

const char kAction[] = "action";
const char kEvent[] = "event";
const char kIntentService[] = "com.webos.service.intent";
const char kParameters[] = "parameters";
const char kReason[] = "reason";
const char kRelaunchEvent[] = "relaunch";
const char kRegisterAppMethod[] = "registerApp";
const char kRegisterAppRequest[] = R"JSON({"subscribe":true})JSON";
const char kTarget[] = "target";
const char kUri[] = "uri";

}  // namespace

namespace pal {
namespace webos {

ApplicationRegistratorDelegateWebOS::ApplicationRegistratorDelegateWebOS(
    std::string application_name,
    RepeatingResponse callback)
    : application_name_(std::move(application_name)),
      callback_(std::move(callback)),
      weak_factory_(this) {

  base::CommandLine* cmd = base::CommandLine::ForCurrentProcess();
  if (!cmd->HasSwitch(switches::kWebOSLS2Name)) {
    LOG(ERROR) << __func__ << "no webos-ls2-name has been provided!";
    return;
  }

  pal::luna::Client::Params params;
  params.name = cmd->GetSwitchValueASCII(switches::kWebOSLS2Name);
  luna_client_ = pal::luna::GetSharedClient(params);

  if (luna_client_ && luna_client_->IsInitialized()) {
    const bool subscribed = luna_client_->SubscribeFromApp(
        luna::GetServiceURI(luna::service_uri::kApplicationManager,
                            kRegisterAppMethod),
        std::string(kRegisterAppRequest), application_name_,
        base::BindRepeating(&ApplicationRegistratorDelegateWebOS::OnResponse,
                            weak_factory_.GetWeakPtr()));
    status_ = subscribed ? Status::kSuccess : Status::kFailed;
  }
}

ApplicationRegistratorDelegateWebOS::~ApplicationRegistratorDelegateWebOS() {}

ApplicationRegistratorDelegate::Status
ApplicationRegistratorDelegateWebOS::GetStatus() const {
  return status_;
}

std::string ApplicationRegistratorDelegateWebOS::GetApplicationName() const {
  return application_name_;
};

void ApplicationRegistratorDelegateWebOS::OnResponse(
    pal::luna::Client::ResponseStatus,
    unsigned,
    const std::string& json) {
  std::optional<base::Value> root(base::JSONReader::Read(json));
  if (!root || !root->is_dict()) {
    LOG(ERROR) << __func__ << "(): Failed to register application.";
    return;
  }

  const base::Value::Dict& root_dict = root->GetDict();
  std::optional<bool> return_value = root_dict.FindBool("returnValue");

  if (return_value.has_value() && !return_value.value()) {
    const std::string* message = root_dict.FindString("errorText");
    LOG(ERROR) << __func__ << "(): Failed to register application. Error: "
               << (message ? *message : "");
    return;
  }

  const std::string* event = root_dict.FindString(kEvent);
  if (!event) {
    LOG(ERROR) << __func__ << "() event field is absent.";
    return;
  }

  std::string options;
  if (*event == kRelaunchEvent) {
    if (!ParseRelaunchEvent(root_dict, &options)) {
      LOG(ERROR) << __func__ << "(): Failed to parse relaunch event";
      return;
    }
  }

  callback_.Run(*event, options);
}

bool ApplicationRegistratorDelegateWebOS::ParseRelaunchEvent(
    const base::Value::Dict& root_dict, std::string* options) {
  if (!options)
    return false;

  const std::string* reason = root_dict.FindString(kReason);

  if (!reason) {
    LOG(ERROR) << __func__ << "() reason field is absent.";
    return false;
  }

  const base::Value::Dict* params_dict = root_dict.FindDict(kParameters);
  if (!params_dict) {
    LOG(ERROR) << __func__ << "() parameters field is absent.";
    return false;
  }

  base::Value::Dict detail_dict;
  if (*reason == kIntentService) {
    const std::string* action = params_dict->FindString(kAction);
    if (!action || action->empty())
      return false;

    const std::string* uri = params_dict->FindString(kUri);
    if (!uri || uri->empty())
      return false;

    detail_dict.Set("action", *action);
    detail_dict.Set("uri", *uri);
  } else {
    const std::string* target_url = params_dict->FindString(kTarget);
    if (target_url && !target_url->empty())
      detail_dict.Set("url", *target_url);
  }

  base::Value::Dict options_dict;
  options_dict.Set("detail", std::move(detail_dict));

  base::JSONWriter::Write(options_dict, options);
  return true;
}

}  // namespace webos
}  // namespace pal
