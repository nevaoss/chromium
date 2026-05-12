// Copyright 2020 LG Electronics, Inc.
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

#include "neva/wam_demo/wam_demo_app_launch_params.h"

namespace wam_demo {

namespace {

const char kApplicationPalmOSDefaultSystemPrefix[] = "com.palm.";
const char kApplicationWebOSDefaultSystemPrefix[] = "com.webos.app";
const char kApplicationWamDemoSystemPrefix[] = "com.webos.app.neva.wam.demo.";

}  // namespace

AppLaunchParams::AppLaunchParams() = default;
AppLaunchParams::~AppLaunchParams() = default;

std::unique_ptr<base::Value::Dict> AppLaunchParams::AsDict(
    const AppLaunchParams& params) {
  auto dict = std::make_unique<base::Value::Dict>();
  dict->Set("appid", params.appid);
  dict->Set("appurl", params.appurl);
  dict->Set("fullscreen", params.fullscreen);
  dict->Set("frameless", params.frameless);
  dict->Set("profile_name", params.profile_name);
  dict->Set("group", params.group);
  dict->Set("group_owner", params.group_owner);
  dict->Set("transparent_background", params.transparent_background);

  base::Value::List injection_list;
  for (const auto& injection : params.injections)
    injection_list.Append(injection);
  dict->Set("injections", std::move(injection_list));
  return dict;
}

std::string GetSystemAppId(const std::string& service_appid) {
  if (service_appid.find(kApplicationPalmOSDefaultSystemPrefix) == 0 ||
      service_appid.find(kApplicationWebOSDefaultSystemPrefix) == 0)
    return service_appid;
  return std::string(kApplicationWamDemoSystemPrefix) + service_appid;
}

std::string GetSystemGroupName(const std::string& service_group) {
  if (service_group.find(kApplicationPalmOSDefaultSystemPrefix) == 0 ||
      service_group.find(kApplicationWebOSDefaultSystemPrefix) == 0)
    return service_group;
  return std::string(kApplicationWamDemoSystemPrefix) + service_group;
}

}  // namespace wam_demo

