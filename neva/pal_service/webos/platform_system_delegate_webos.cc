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

#include "neva/pal_service/webos/platform_system_delegate_webos.h"

#include <cstdio>

#include "base/files/file_util.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/logging.h"
#include "neva/pal_service/luna/luna_names.h"

namespace pal {

namespace webos {

using namespace luna;

PlatformSystemDelegateWebOS::PlatformSystemDelegateWebOS() {
  Client::Params params;
  params.name =
      luna::GetServiceNameWithRandSuffix(service_name::kChromiumPlatformSystem);
  luna_client_ = CreateClient(params);
}

PlatformSystemDelegateWebOS::~PlatformSystemDelegateWebOS() = default;

void PlatformSystemDelegateWebOS::Initialize() {
  if (GetLunaClient() || GetLunaClient()->IsInitialized())
    ConnectToServices();
}

std::string PlatformSystemDelegateWebOS::GetCountry() const {
  std::stringstream result;
  std::string local_country;
  std::string smart_service_country;

  auto it = registry_.find("LocalCountry");
  if (it != registry_.end())
    local_country = it->second;

  it = registry_.find("SmartServiceCountry");
  if (it != registry_.end())
    smart_service_country = it->second;

  result << "{ \"country\": \"" << local_country
         << "\", \"smartServiceCountry\": \"" << smart_service_country
         << "\" }";

  return result.str();
}

std::string PlatformSystemDelegateWebOS::GetDeviceInfoJSON() const {
  base::Value::Dict dict;

  std::string model_name;
  if (GetKeyValue("ModelName", model_name))
    dict.Set("modelName", model_name);

  std::string firmware_version;
  if (GetKeyValue("FirmwareVersion", firmware_version)) {
    dict.Set("platformVersion", firmware_version);
    int major, minor, dot;
    const int fields_count =
        std::sscanf(firmware_version.c_str(), "%d.%d.%d", &major, &minor, &dot);
    if (fields_count != 3)
      major = minor = dot = -1;
    dict.Set("platformVersionMajor", major);
    dict.Set("platformVersionMinor", minor);
    dict.Set("platformVersionDot", dot);
  }

  dict.Set("screenWidth", GetScreenWidth());
  dict.Set("screenHeight", GetScreenHeight());

  std::string panel_type;
  if (GetKeyValue("panelType", panel_type))
    dict.Set("panelType", panel_type);

  std::string info;
  if (base::JSONWriter::Write(dict, &info))
    return info;
  return std::string("{}");
}

int PlatformSystemDelegateWebOS::GetScreenWidth() const {
  return 0;
}

int PlatformSystemDelegateWebOS::GetScreenHeight() const {
  return 0;
}

std::string PlatformSystemDelegateWebOS::GetLocale() const {
  auto it = registry_.find("SystemLanguage");
  if (it != registry_.end())
    return it->second;
  return std::string();
}

std::string PlatformSystemDelegateWebOS::GetLocaleRegion() const {
  return std::string("US");
}

std::string PlatformSystemDelegateWebOS::GetResource(
    const std::string& path) const {
  std::string file_str;
  if (!base::ReadFileToString(base::FilePath(path), &file_str)) {
    LOG(ERROR) << __func__ << ": Failed to read resource file: " << path;
    return std::string();
  }
  return file_str;
}

bool PlatformSystemDelegateWebOS::IsMinimal() const {
  return base::PathExists(base::FilePath("/var/luna/preferences/ran-firstuse"));
}

bool PlatformSystemDelegateWebOS::GetHightContrast() const {
  auto it = registry_.find("HightContrast");
  return (it != registry_.end()) && (it->second == std::string("on"));
}

luna::Client* PlatformSystemDelegateWebOS::GetLunaClient() {
  return luna_client_.get();
}

void PlatformSystemDelegateWebOS::SetKeyValue(const std::string& key,
                                              std::string value) {
  registry_[key] = std::move(value);
}

bool PlatformSystemDelegateWebOS::GetKeyValue(const std::string& key,
                                              std::string& value) const {
  auto it = registry_.find(key);
  if (it == registry_.end())
    return false;
  value = it->second;
  return true;
}

void PlatformSystemDelegateWebOS::ConnectToServices() {
  if (!GetLunaClient() || !GetLunaClient()->IsInitialized())
    return;

  std::string settings_service_params = R"JSON({
        "subscribe" : true,
        "serviceName": "com.webos.settingsservice"
  })JSON";

  GetLunaClient()->Subscribe(
      luna::GetServiceURI(luna::service_uri::kPalmBus,
                          "signal/registerServerStatus"),
      std::move(settings_service_params),
      base::BindRepeating(
          &PlatformSystemDelegateWebOS::SettingsServiceConnected,
          base::Unretained(this)));
}

void PlatformSystemDelegateWebOS::LoadLocalePreferences() {
  const std::string path_str = "/var/luna/preferences/localeInfo";
  std::string json;
  if (base::ReadFileToString(base::FilePath(path_str), &json)) {
    std::optional<base::Value> root(base::JSONReader::Read(json));
    if (!root || !root->is_dict())
      return;

    const base::Value::Dict& dict = root->GetDict();
    const std::string* system_language =
        dict.FindStringByDottedPath("localeInfo.locales.UI");
    if (system_language)
      registry_["SystemLanguage"] = *system_language;

    const std::string* country = dict.FindStringByDottedPath("country");
    if (country)
      registry_["LocalCountry"] = *country;

    const std::string* smart_service_country =
        dict.FindStringByDottedPath("smartServiceCountryCode3");
    if (smart_service_country)
      registry_["SmartServiceCountry"] = *smart_service_country;
  }
}

void PlatformSystemDelegateWebOS::RequestSystemSettings() {
  std::string locale_params = R"JSON(
    {"subscribe" : "true", "keys" : {"localInfo"}}
  )JSON";

  GetLunaClient()->Subscribe(
      luna::GetServiceURI("com.webos.settingsservice", "getSystemSettings"),
      std::move(locale_params),
      base::BindRepeating(
          &PlatformSystemDelegateWebOS::OnLocalePreferencesCallback,
          base::Unretained(this)));

  std::string hight_contrast_params = R"JSON(
    {"subscribe" : "true", "category": "option", "keys" : {"hightContrast"}}
  )JSON";

  GetLunaClient()->Subscribe(
      luna::GetServiceURI("com.webos.settingsservice", "getSystemSettings"),
      std::move(hight_contrast_params),
      base::BindRepeating(&PlatformSystemDelegateWebOS::OnHightContrastCallback,
                          base::Unretained(this)));

  std::string option_params = R"JSON({
     "subscribe" : "true",
     "category" : "option",
     "keys" : {"country", "smartServiceCountryCode3",
               "audioGuidance", "screenRotation"})JSON";

  GetLunaClient()->Subscribe(
      luna::GetServiceURI("com.webos.settingsservice", "getSystemSettings"),
      std::move(option_params),
      base::BindRepeating(&PlatformSystemDelegateWebOS::OnSystemOptionCallback,
                          base::Unretained(this)));
}

void PlatformSystemDelegateWebOS::SettingsServiceConnected(
    luna::Client::ResponseStatus,
    unsigned,
    const std::string& json) {
  std::optional<base::Value> root(base::JSONReader::Read(json));
  if (!root || !root->is_dict())
    return;

  const base::Value::Dict& dict(root->GetDict());
  if (dict.FindBool("connected").value_or(false))
    RequestSystemSettings();
}

void PlatformSystemDelegateWebOS::OnLocalePreferencesCallback(
    luna::Client::ResponseStatus,
    unsigned,
    const std::string& json) {
  std::optional<base::Value> root(base::JSONReader::Read(json));
  if (!root || !root->is_dict())
    return;

  const base::Value::Dict& dict = root->GetDict();
  const std::string* system_language =
      dict.FindStringByDottedPath("settings.localeInfo.locales.UI");
  if (system_language)
    registry_["SystemLanguage"] = *system_language;

  // TODO(pikulik): Notify System Language changed
}

void PlatformSystemDelegateWebOS::OnHightContrastCallback(
    luna::Client::ResponseStatus,
    unsigned,
    const std::string& json) {
  std::optional<base::Value> root(base::JSONReader::Read(json));
  if (!root || !root->is_dict())
    return;

  const base::Value::Dict& dict = root->GetDict();
  const std::string* hight_contrast =
      dict.FindStringByDottedPath("settings.highContrast");
  if (hight_contrast)
    registry_["HightContrast"] = *hight_contrast;

  // TODO(pikulik): Notify Hight Contrast changed
}

void PlatformSystemDelegateWebOS::OnSystemOptionCallback(
    luna::Client::ResponseStatus,
    unsigned,
    const std::string& json) {
  std::optional<base::Value> root(base::JSONReader::Read(json));
  if (!root || !root->is_dict())
    return;

  const base::Value::Dict& dict = root->GetDict();
  const std::string* country = dict.FindStringByDottedPath("settings.country");
  if (country)
    registry_["LocalCountry"] = *country;

  const std::string* smart_service_country =
      dict.FindStringByDottedPath("settings.smartServiceCountryCode3");
  if (smart_service_country)
    registry_["SmartServiceCountry"] = *smart_service_country;

  const std::string* screen_rotation =
      dict.FindStringByDottedPath("settings.ScreenRotation");
  if (screen_rotation)
    registry_["ScreenRotation"] = *screen_rotation;

  // TODO(pikulik): Looks like "audioGuidance" could be handled here too
}

}  // namespace webos

}  // namespace pal
