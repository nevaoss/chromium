// Copyright 2025 LG Electronics, Inc.
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

#include "neva/pal_service/webos/launchpoint_delegate_webos.h"

#include "base/command_line.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/logging.h"
#include "base/values.h"
#include "neva/pal_service/luna/luna_names.h"
#include "neva/pal_service/webos/switches.h"

namespace pal {
namespace webos {

const char kAddLaunchPointMethod[] = "addLaunchPoint";
const char kEnactBrowserApplicationId[] = "com.webos.app.enactbrowser";

LaunchPointDelegateWebOS::LaunchPointDelegateWebOS() : weak_factory_(this) {
  weak_ptr_this_ = weak_factory_.GetWeakPtr();

  base::CommandLine* cmd = base::CommandLine::ForCurrentProcess();
  if (!cmd->HasSwitch(switches::kWebOSLS2Name)) {
    LOG(ERROR) << __func__ << "no webos-ls2-name has been provided!";
    return;
  }

  pal::luna::Client::Params params;
  params.name = cmd->GetSwitchValueASCII(switches::kWebOSLS2Name);
  luna_client_ = pal::luna::GetSharedClient(params);
}

LaunchPointDelegateWebOS::~LaunchPointDelegateWebOS() = default;

void LaunchPointDelegateWebOS::AddLaunchPoint(
    const std::string& launch_point_info,
    OnceResponse callback) {
  // Read launch_point_info string as json for parsing.
  std::optional<base::Value> root(base::JSONReader::Read(
      launch_point_info, base::JSON_PARSE_CHROMIUM_EXTENSIONS));
  if (!root || !root->is_dict()) {
    return;
  }

  const base::DictValue& dict = root->GetDict();
  const std::string* title = dict.FindString("title");
  const std::string* url = dict.FindString("url");

  if (title && url && !title->empty() && !url->empty()) {
    LOG(INFO) << __func__
              << " read launch point info from Browser: title=" << *title
              << ", url=" << *url;
  } else {
    LOG(ERROR) << __func__
               << " wrong launch point info value: " << launch_point_info;
    return;
  }

  // Write json to call 'addLaunchPoint' luna command.
  base::DictValue value, params;
  params.Set("target", *url);
  value.Set("id", kEnactBrowserApplicationId);
  value.Set("title", *title);
  value.Set("params", std::move(params));
  std::string launch_point_params;

  if (base::JSONWriter::Write(value, &launch_point_params)) {
    LOG(INFO) << __func__
              << ": write launch point info to json: " << launch_point_params;

    luna_client_->Call(
        pal::luna::GetServiceURI(
            pal::luna::service_uri::kServiceApplicationManager,
            kAddLaunchPointMethod),
        launch_point_params,
        base::BindOnce(&LaunchPointDelegateWebOS::OnAddLaunchPoint,
                       weak_ptr_this_, std::move(callback)),
        std::string("{}"));
  }
}

void LaunchPointDelegateWebOS::OnAddLaunchPoint(OnceResponse callback,
                                                luna::Client::ResponseStatus,
                                                unsigned,
                                                const std::string& json) {
  std::optional<base::Value> root(
      base::JSONReader::Read(json, base::JSON_PARSE_CHROMIUM_EXTENSIONS));
  if (!root || !root->is_dict()) {
    return;
  }

  const base::DictValue& dict = root->GetDict();
  std::optional<bool> return_value = dict.FindBool("returnValue");

  if (return_value.has_value()) {
    std::string message =
        (*return_value)
            ? " launch point added, response: "
            : " invalid launch_point_info, launch point not added, response: ";

    const std::string& launch_point_id =
        (*return_value) ? *dict.FindString("launchPointId") : "";
    std::optional<int> error_code =
        (*return_value) ? 0 : dict.FindInt("errorCode");
    const std::string& error_text =
        (*return_value) ? "" : *dict.FindString("errorText");

    LOG(INFO) << __func__ << message << json;

    std::move(callback).Run(std::move(*return_value),
                            std::move(launch_point_id), std::move(*error_code),
                            std::move(error_text));
  } else {
    LOG(ERROR) << __func__
               << " 'returnValue' is missing or not a boolean, response: "
               << json;
  }
}

}  // namespace webos
}  // namespace pal
