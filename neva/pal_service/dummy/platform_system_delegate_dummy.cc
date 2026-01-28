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

#include "neva/pal_service/dummy/platform_system_delegate_dummy.h"

#include "base/files/file_util.h"
#include "base/logging.h"

namespace pal {

namespace dummy {

PlatformSystemDelegateDummy::PlatformSystemDelegateDummy() = default;

PlatformSystemDelegateDummy::~PlatformSystemDelegateDummy() = default;

void PlatformSystemDelegateDummy::Initialize() {}

std::string PlatformSystemDelegateDummy::GetCountry() const {
  return std::string("{}");
}

std::string PlatformSystemDelegateDummy::GetDeviceInfoJSON() const {
  return std::string("{}");
}

int PlatformSystemDelegateDummy::GetScreenWidth() const {
  return 0;
}

int PlatformSystemDelegateDummy::GetScreenHeight() const {
  return 0;
}

std::string PlatformSystemDelegateDummy::GetLocale() const {
  return std::string();
}

std::string PlatformSystemDelegateDummy::GetLocaleRegion() const {
  return std::string("US");
}

std::string PlatformSystemDelegateDummy::GetResource(
    const std::string& path) const {
  std::string file_str;
  if (!base::ReadFileToString(base::FilePath(path), &file_str)) {
    LOG(ERROR) << __func__ << ": Failed to read resource file: " << path;
    return std::string();
  }
  return file_str;
}

bool PlatformSystemDelegateDummy::IsMinimal() const {
  return false;
}

bool PlatformSystemDelegateDummy::GetHightContrast() const {
  return false;
}

}  // namespace dummy

}  // namespace pal
