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

#include "neva/injection/renderer/webossystem/webossystem_datamanager.h"

namespace injections {

const std::vector<std::string> WebOSSystemDataManager::cached_data_keys_ = {
  "activityId",
  "country",
  "currentCountryGroup",
  "deviceInfo",
  "folderPath",
  "highContrast",
  "identifier",
  "isMinimal",
  "launchParams",
  "locale",
  "localeRegion",
  "phoneRegion",
  "screenOrientation",
  "timeFormat",
  "timeZone",
  "tvSystemName",
  "devicePixelRatio",
  "trustLevel"
};

WebOSSystemDataManager::WebOSSystemDataManager(const std::string& json) {
  DoInitialize(json);
}

WebOSSystemDataManager::~WebOSSystemDataManager() = default;

void WebOSSystemDataManager::DoInitialize(const std::string& json) {
  if (!initialized_ && !json.empty()) {
    Initialize(json, cached_data_keys_);
    initialized_ = true;
  }
}

}  // namespace injections
