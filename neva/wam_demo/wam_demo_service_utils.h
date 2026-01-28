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


#ifndef NEVA_WAM_DEMO_WAM_DEMO_SERVICE_UTILS_H_
#define NEVA_WAM_DEMO_WAM_DEMO_SERVICE_UTILS_H_

#include <string>
#include <vector>

#include "neva/app_runtime/public/app_runtime_constants.h"
#include "neva/app_runtime/public/webapp_window_base.h"

namespace wam_demo {

bool UnpackGeneralParams(const std::string& src,
                         std::string& appid,
                         std::string& cmd,
                         std::string& appurl);
bool UnpackLayoutParams(const std::string& src,
                        int& x, int& y, int& w, int& h);
bool UnpackInjections(const std::string& src,
                      std::vector<std::string>& injections);
bool UnpackBackgroundColor(const std::string& src, int& r, int& g, int& b);

bool UnpackBool(const std::string& src, const char* name, bool& param);

bool UnpackUInt(const std::string& src, const char* name, unsigned& param);

bool UnpackFloat(const std::string& src, const char* name, float& param);

bool UnpackString(const std::string& src, const char* name, std::string& param);

bool UnpackViewportParams(const std::string& src, int& w, int& h);

bool UnpackWindowSize(const std::string& src, int& w, int& h);

bool GetWidgetStateFromInteger(int s, neva_app_runtime::WidgetState& state);

bool UnpackWindowState(const std::string& src, neva_app_runtime::WidgetState& s);

const char* GetWidgetStateString(neva_app_runtime::WidgetState state);

bool UnpackInputRegion(const std::string& src,
                       const char* name,
                       neva_app_runtime::WebAppWindowBase::Region& input_region,
                       int window_width,
                       int window_height);

}  // namespace wam_demo

#endif  // NEVA_WAM_DEMO_WAM_DEMO_SERVICE_UTILS_H_
