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

#ifndef NEVA_WAM_DEMO_WAM_DEMO_APP_LAUNCH_PARAMS_H_
#define NEVA_WAM_DEMO_WAM_DEMO_APP_LAUNCH_PARAMS_H_

#include <memory>
#include <string>
#include <vector>

#include "base/time/time.h"
#include "base/values.h"
#include "content/public/common/main_function_params.h"
#include "neva/app_runtime/public/app_runtime_constants.h"

namespace wam_demo {

struct AppLaunchParams {
  AppLaunchParams();
  ~AppLaunchParams();

  static std::unique_ptr<base::Value::Dict> AsDict(
      const AppLaunchParams& params);

  std::string appid;
  std::string appurl;
  bool fullscreen = false;
  bool frameless = false;
  int layout_x = 0;
  int layout_y = 0;
  int layout_width = 0;
  int layout_height = 0;
  std::string profile_name;
  int viewport_width = 0;
  int viewport_height = 0;
  bool transparent_background = false;
  std::vector<std::string> injections;
  std::string group;
  bool group_owner = false;
};

std::string GetSystemAppId(const std::string& appid);

std::string GetSystemGroupName(const std::string& group);

}  // namespace wam_demo

#endif  // NEVA_WAM_DEMO_WAM_DEMO_APP_LAUNCH_PARAMS_H_
