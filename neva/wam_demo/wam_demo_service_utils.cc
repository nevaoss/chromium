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

#include "neva/wam_demo/wam_demo_service_utils.h"

#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "neva/emulator/emulator_data_source.h"
#include "neva/emulator/emulator_urls.h"
#include "neva/wam_demo/wam_demo_emulator_commands.h"

namespace wam_demo {

bool UnpackGeneralParams(const std::string& src,
                         std::string& appid,
                         std::string& cmd,
                         std::string& appurl) {
  emulator::ResponseArgs args_vector;
  args_vector.push_back({argument::kAppId, &appid});
  args_vector.push_back({argument::kCmd, &cmd});
  args_vector.push_back({argument::kUrl, &appurl});
  return emulator::EmulatorDataSource::GetResponseParams(args_vector, src);
}

bool UnpackLayoutParams(const std::string& src,
                        int& x, int& y, int& w, int& h) {
  std::string width;
  std::string height;
  std::string pos_x;
  std::string pos_y;

  emulator::ResponseArgs args_vector;
  args_vector.push_back({argument::kWidth, &width});
  args_vector.push_back({argument::kHeight, &height});
  args_vector.push_back({argument::kPosX, &pos_x});
  args_vector.push_back({argument::kPosY, &pos_y});

  if (!emulator::EmulatorDataSource::GetResponseParams(args_vector, src))
    return false;

  return base::StringToInt(pos_x, &x) && base::StringToInt(pos_y, &y) &&
      base::StringToInt(width, &w) && base::StringToInt(height, &h);
}

bool UnpackInjections(const std::string& src,
                      std::vector<std::string>& injections) {
  emulator::ResponseArgs args_vector;
  std::string csv;
  args_vector.push_back({"injections", &csv});
  if (emulator::EmulatorDataSource::GetResponseParams(args_vector, src)) {
    std::vector<std::string> res = base::SplitString(
      csv, ",;", base::TRIM_WHITESPACE, base::SPLIT_WANT_NONEMPTY);
    if (!res.empty()) {
      injections = std::move(res);
      return true;
    }
  }
  return false;
}

bool UnpackBackgroundColor(const std::string& src, int& r, int& g, int& b) {
  std::string red;
  std::string green;
  std::string blue;

  emulator::ResponseArgs args_vector;
  args_vector.push_back({argument::kRedColor, &red});
  args_vector.push_back({argument::kGreenColor, &green});
  args_vector.push_back({argument::kBlueColor, &blue});

  if (!emulator::EmulatorDataSource::GetResponseParams(args_vector, src))
    return false;

  return base::StringToInt(red, &r) && base::StringToInt(green, &g) &&
      base::StringToInt(blue, &b);
}

bool UnpackBool(const std::string& src, const char* name, bool& param) {
  std::string str_param;
  emulator::ResponseArgs args_vector;
  args_vector.push_back({name, &str_param});

  if (!emulator::EmulatorDataSource::GetResponseParams(args_vector, src))
    return false;

  const std::string upper_str_param = base::ToUpperASCII(str_param);
  if (upper_str_param == std::string("TRUE")) {
    param = true;
    return true;
  }

  if (upper_str_param == std::string("FALSE")) {
    param = false;
    return true;
  }

  param = false;
  return false;
}

bool UnpackUInt(const std::string& src, const char* name, unsigned& param) {
  std::string str_param;
  emulator::ResponseArgs args_vector;
  args_vector.push_back({name, &str_param});

  if (!emulator::EmulatorDataSource::GetResponseParams(args_vector, src))
    return false;

  return base::StringToUint(str_param, &param);
}

bool UnpackFloat(const std::string& src, const char* name, float& param) {
  std::string str_param;
  emulator::ResponseArgs args_vector;
  args_vector.push_back({name, &str_param});

  if (!emulator::EmulatorDataSource::GetResponseParams(args_vector, src))
    return false;

  double res = 0;
  if (base::StringToDouble(str_param, &res)) {
    param = static_cast<float>(res);
    return true;
  }

  return false;
}

bool UnpackString(const std::string& src, const char* name, std::string& param) {
  emulator::ResponseArgs args_vector;
  args_vector.push_back({name, &param});

  if (!emulator::EmulatorDataSource::GetResponseParams(args_vector, src))
    return false;

  if (param.empty())
    return false;

  return true;
}

bool UnpackViewportParams(const std::string& src,
                          int& w, int& h) {
  std::string width;
  std::string height;

  emulator::ResponseArgs args_vector;
  args_vector.push_back({argument::kViewportWidth, &width});
  args_vector.push_back({argument::kViewportHeight, &height});

  if (!emulator::EmulatorDataSource::GetResponseParams(args_vector, src))
    return false;

  return base::StringToInt(width, &w) && base::StringToInt(height, &h);
}

bool UnpackWindowSize(const std::string& src, int& w, int& h) {
  std::string width, height;

  emulator::ResponseArgs args_vector;
  args_vector.push_back({argument::kWindowWidth, &width});
  args_vector.push_back({argument::kWindowHeight, &height});

  if (!emulator::EmulatorDataSource::GetResponseParams(args_vector, src))
    return false;

  return base::StringToInt(width, &w) && base::StringToInt(height, &h);
}

bool GetWidgetStateFromInteger(int s, neva_app_runtime::WidgetState& state) {
  bool result = true;
  switch (s) {
    case 3:
      state = neva_app_runtime::WidgetState::FULLSCREEN;
      break;
    case 4:
      state = neva_app_runtime::WidgetState::MAXIMIZED;
      break;
    case 5:
      state = neva_app_runtime::WidgetState::MINIMIZED;
      break;
    default:
      state = neva_app_runtime::WidgetState::UNINITIALIZED;
      result = false;
      break;
  }
  return result;
}

bool UnpackWindowState(const std::string& src, neva_app_runtime::WidgetState& s) {
  std::string state;

  emulator::ResponseArgs args_vector;
  args_vector.push_back({argument::kWindowState, &state});

  if (!emulator::EmulatorDataSource::GetResponseParams(args_vector, src))
    return false;

  int value = 0;

  return base::StringToInt(state, &value) && GetWidgetStateFromInteger(value, s);
}

const char* GetWidgetStateString(neva_app_runtime::WidgetState state) {
  switch (state) {
    case neva_app_runtime::WidgetState::SHOW:
      return "SHOW";
    case neva_app_runtime::WidgetState::HIDE:
      return "HIDE";
    case neva_app_runtime::WidgetState::FULLSCREEN:
      return "FULLSCREEN";
    case neva_app_runtime::WidgetState::MAXIMIZED:
      return "MAXIMIZED";
    case neva_app_runtime::WidgetState::MINIMIZED:
      return "MINIMIZED";
    case neva_app_runtime::WidgetState::RESTORE:
      return "RESTORE";
    case neva_app_runtime::WidgetState::ACTIVE:
      return "ACTIVE";
    case neva_app_runtime::WidgetState::INACTIVE:
      return "INACTIVE";
    case neva_app_runtime::WidgetState::RESIZE:
      return "RESIZE";
    case neva_app_runtime::WidgetState::DESTROYED:
      return "DESTROYED";
    default:
      return "UNINITIALIZED";
  }
}

bool UnpackInputRegion(const std::string& src,
                       const char* name,
                       neva_app_runtime::WebAppWindowBase::Region& input_region,
                       int window_width,
                       int window_height) {
  std::string region;
  emulator::ResponseArgs args_vector;
  args_vector.push_back({name, &region});

  if (!emulator::EmulatorDataSource::GetResponseParams(args_vector, src))
    return false;

  int res = 0;
  if (base::StringToInt(region, &res)) {
    switch (res) {
      case 1:
        input_region = neva_app_runtime::WebAppWindowBase::Region{
            0, 0, window_width / 2, window_height / 2};
        break;
      case 2:
        input_region = neva_app_runtime::WebAppWindowBase::Region{
            window_width / 2, 0, window_width / 2, window_height / 2};
        break;
      case 3:
        input_region = neva_app_runtime::WebAppWindowBase::Region{
            0, window_height / 2, window_width / 2, window_height / 2};
        break;
      case 4:
        input_region = neva_app_runtime::WebAppWindowBase::Region{
            window_width / 2, window_height / 2, window_width / 2,
            window_height / 2};
        break;
      default:
        return false;
    }
    return true;
  }

  return false;
}

}  // namespace wam_demo
