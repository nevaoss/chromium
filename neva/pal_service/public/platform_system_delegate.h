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

#ifndef NEVA_PAL_SERVICE_PUBLIC_PLATFORM_SYSTEM_DELEGATE_H_
#define NEVA_PAL_SERVICE_PUBLIC_PLATFORM_SYSTEM_DELEGATE_H_

#include <string>

#include "base/component_export.h"

namespace pal {

class COMPONENT_EXPORT(PAL_SERVICE) PlatformSystemDelegate {
 public:
  virtual ~PlatformSystemDelegate() {}

  virtual void Initialize() = 0;
  virtual std::string GetCountry() const = 0;
  virtual std::string GetDeviceInfoJSON() const = 0;
  virtual int GetScreenWidth() const = 0;
  virtual int GetScreenHeight() const = 0;
  virtual std::string GetLocale() const = 0;
  virtual std::string GetLocaleRegion() const = 0;
  virtual std::string GetResource(const std::string& path) const = 0;
  virtual bool IsMinimal() const = 0;
  virtual bool GetHightContrast() const = 0;
};

}  // namespace pal

#endif  // NEVA_PAL_SERVICE_PUBLIC_PLATFORM_SYSTEM_DELEGATE_H_
