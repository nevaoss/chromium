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

#ifndef NEVA_PAL_SERVICE_WEBOS_PLATFORM_SYSTEM_DELEGATE_WEBOS_H_
#define NEVA_PAL_SERVICE_WEBOS_PLATFORM_SYSTEM_DELEGATE_WEBOS_H_

#include <map>
#include <string>

#include "base/functional/callback.h"
#include "neva/pal_service/luna/luna_client.h"
#include "neva/pal_service/public/platform_system_delegate.h"

namespace pal {

namespace webos {

class PlatformSystemDelegateWebOS : public PlatformSystemDelegate {
 public:
  PlatformSystemDelegateWebOS();
  ~PlatformSystemDelegateWebOS() override;

  PlatformSystemDelegateWebOS(const PlatformSystemDelegateWebOS&) = delete;
  PlatformSystemDelegateWebOS& operator=(const PlatformSystemDelegateWebOS&) =
      delete;

  void Initialize() override;
  std::string GetCountry() const override;
  std::string GetDeviceInfoJSON() const override;
  int GetScreenWidth() const override;
  int GetScreenHeight() const override;
  std::string GetLocale() const override;
  std::string GetLocaleRegion() const override;
  std::string GetResource(const std::string& path) const override;
  bool IsMinimal() const override;
  bool GetHightContrast() const override;

 protected:
  luna::Client* GetLunaClient();
  void SetKeyValue(const std::string& key, std::string value);
  bool GetKeyValue(const std::string& key, std::string& value) const;
  void ConnectToServices();

 private:
  void LoadLocalePreferences();
  void RequestSystemSettings();

  void SettingsServiceConnected(luna::Client::ResponseStatus status,
                                unsigned token,
                                const std::string& json);

  void OnLocalePreferencesCallback(luna::Client::ResponseStatus status,
                                   unsigned token,
                                   const std::string& json);

  void OnHightContrastCallback(luna::Client::ResponseStatus status,
                               unsigned token,
                               const std::string& json);

  void OnSystemOptionCallback(luna::Client::ResponseStatus status,
                              unsigned token,
                              const std::string& json);

  std::unique_ptr<luna::Client> luna_client_;
  std::map<std::string, std::string> registry_;
};

}  // namespace webos

}  // namespace pal

#endif  // NEVA_PAL_SERVICE_WEBOS_PLATFORM_SYSTEM_DELEGATE_WEBOS_H_
