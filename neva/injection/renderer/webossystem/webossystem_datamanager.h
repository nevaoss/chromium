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

#ifndef NEVA_INJECTION_WEBOSSYSTEM_WEBOSSYSTEM_DATAMANAGER_H_
#define NEVA_INJECTION_WEBOSSYSTEM_WEBOSSYSTEM_DATAMANAGER_H_

#include "neva/injection/renderer/injection_data_manager.h"

#include <string>
#include <vector>

namespace injections {

class WebOSSystemDataManager : public InjectionDataManager {
 public:
  WebOSSystemDataManager(const std::string& json);
  ~WebOSSystemDataManager() override;

  bool GetInitializedStatus() const {
    return initialized_;
  }

  void SetInitializedStatus(bool status) {
    initialized_ = status;
  }

  void DoInitialize(const std::string& json);

 private:
  static const std::vector<std::string> cached_data_keys_;
  bool initialized_ = false;
};

}  // namespace injections

#endif  // NEVA_INJECTION_WEBOSSYSTEM_WEBOSSYSTEM_DATAMANAGER_H_
