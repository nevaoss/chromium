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

#include "neva/pal_service/luna/luna_names.h"

#include "base/logging.h"
#include "base/rand_util.h"

#include <stdlib.h>
#include <unistd.h>

namespace pal {
namespace luna {

namespace {

const char kLunaScheme[] = "luna://";
const char KPalmScheme[] = "palm://";

}  // namespace

namespace service_uri {

const char kAudio[] = "com.webos.audio";
const char kSettings[] = "com.webos.settingsservice";
const char kMemoryManager[] = "com.webos.memorymanager";
const char kPalmBus[] = "com.palm.bus";
const char kApplicationManager[] = "com.webos.applicationManager";
const char kServiceApplicationManager[] =
    "com.webos.service.applicationmanager";
const char kServiceMemoryManager[] = "com.webos.service.memorymanager";
const char kNotification[] = "com.webos.notification";

}  // namespace service_uri

namespace service_name {

const char kChromiumMedia[] = "com.webos.chromium.media";
const char kChromiumMemory[] = "com.webos.chromium.memory";
const char kChromiumPlatformSystem[] = "com.webos.chromium.platform.system";
const char kSettingsClient[] = "com.webos.settingsservice.client";
const char kNotificationClient[] = "com.webos.notification.client";
const char kApplicationManagerClient[] = "com.webos.applicationManager.client";

}  // namespace service_name

std::string GetServiceURI(const std::string& uri, const std::string& action) {
  std::string result(kLunaScheme);
  return result.append(uri).append("/").append(action);
}

std::string GetServiceName(const std::string& name,
                           int suffix,
                           const std::string& delimiter) {
  std::string result(name);
  return result.append(delimiter).append(std::to_string(suffix));
}

std::string GetServiceNameWithRandSuffix(const std::string& name,
                                         const std::string& delimiter) {
  return GetServiceName(name, base::RandIntInclusive(10000, 99999), delimiter);
}

std::string GetServiceNameWithPID(const std::string& name,
                                  const std::string& delimiter) {
  return GetServiceName(name, getpid(), delimiter);
}

}  // namespace luna
}  // namespace pal
