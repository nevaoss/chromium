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

#include "neva/pal_service/platform_system_handler.h"

#include <set>

#include "base/compiler_specific.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "neva/pal_service/pal_platform_factory.h"
#include "neva/pal_service/platform_system_handler_extension.h"
#include "neva/pal_service/public/platform_application_delegate.h"
#include "neva/pal_service/public/platform_system_delegate.h"

namespace pal {

namespace messages {

const char kActivate[] = "activate";
const char kActivityId[] = "activityId";
const char kAddBannerMessage[] = "addBannerMessage";
const char kAddNewContentIndicator[] = "addNewContentIndicator";
const char kClearBannerMessages[] = "clearBannerMessages";
const char kContainerReady[] = "containerReady";
const char kCountry[] = "country";
const char kCurrentCountryGroup[] = "currentCountryGroup";
const char kCursorVisibility[] = "cursorVisibility";
const char kDeactivate[] = "deactivate";
const char kDeviceInfo[] = "deviceInfo";
const char kDevicePixelRatio[] = "devicePixelRatio";
const char kEnableFullScreenMode[] = "enableFullScreenMode";
const char kFocusLayer[] = "focusLayer";
const char kFocusOwner[] = "focusOwner";
const char kFolderPath[] = "folderPath";
const char kGetResource[] = "getResource";
const char kHighContrast[] = "highContrast";
const char kHide[] = "hide";
const char kHideCursor[] = "hideCursor";
const char kIdentifier[] = "identifier";
const char kInitialize[] = "initialize";
const char kIsActivated[] = "highContrast";
const char kIsKeyboardVisible[] = "isKeyboardVisible";
const char kIsMinimal[] = "isMinimal";
const char kKeepAlive[] = "keepAlive";
const char kKeyboardHide[] = "keyboardHide";
const char kKeyboardShow[] = "keyboardShow";
const char kLaunchParams[] = "launchParams";
const char kLocale[] = "locale";
const char kLocaleRegion[] = "localeRegion";
const char kOnCloseNotify[] = "onCloseNotify";
const char kPaste[] = "paste";
const char kPhoneRegion[] = "phoneRegion";
const char kPlatformBack[] = "platformBack";
const char kPmLogString[] = "PmLogString";
const char kPmLogInfoWithClock[] = "PmLogInfoWithClock";
const char kRemoveBannerMessage[] = "removeBannerMessage";
const char kRemoveNewContentIndicator[] = "removeNewContentIndicator";
const char kServiceCall[] = "serviceCall";
const char kSetCursor[] = "setCursor";
const char kSetKeyMask[] = "setKeyMask";
const char kSetLoadErrorPolicy[] = "setLoadErrorPolicy";
const char kSetInputRegion[] = "setInputRegion";
const char kSetManualKeyboardEnabled[] = "setManualKeyboardEnabled";
const char kSetWindowOrientation[] = "setWindowOrientation";
const char kSetWindowProperty[] = "setWindowProperty";
const char kScreenOrientation[] = "screenOrientation";
const char kSimulateMouseClick[] = "simulateMouseClick";
const char kStagePreparing[] = "stagePreparing";
const char kStageReady[] = "stageReady";
const char kTimeFormat[] = "timeFormat";
const char kTrustLevel[] = "trustLevel";
const char kTvSystemName[] = "tvSystemName";
const char kWindowOrientation[] = "windowOrientation";

std::set<std::string> unhandled = {
    kActivate,
    kActivityId,
    kAddBannerMessage,
    kAddNewContentIndicator,
    kClearBannerMessages,
    kContainerReady,
    kCurrentCountryGroup,
    kCursorVisibility,
    kDeactivate,
    kEnableFullScreenMode,
    kFocusLayer,
    kFocusOwner,
    kHighContrast,
    kHide,
    kHideCursor,
    kIsActivated,
    kIsKeyboardVisible,
    kKeepAlive,
    kKeyboardHide,
    kKeyboardShow,
    kOnCloseNotify,
    kPaste,
    kPhoneRegion,
    kPlatformBack,
    kPmLogString,
    kPmLogInfoWithClock,
    kRemoveBannerMessage,
    kRemoveNewContentIndicator,
    kServiceCall,
    kSetCursor,
    kSetKeyMask,
    kSetLoadErrorPolicy,
    kSetInputRegion,
    kSetManualKeyboardEnabled,
    kSetWindowOrientation,
    kSetWindowProperty,
    kScreenOrientation,
    kSimulateMouseClick,
    kStagePreparing,
    kStageReady,
    kTimeFormat,
    kTvSystemName,
    kWindowOrientation
};

}  // namespace message

PlatformSystemHandler::PlatformSystemHandler(
    PlatformApplicationDelegate& application_delegate,
    PlatformSystemDelegate& system_delegate)
    : application_delegate_(application_delegate),
      system_delegate_(system_delegate) {
  system_delegate.Initialize();
}

PlatformSystemHandler::~PlatformSystemHandler() = default;

std::string PlatformSystemHandler::GetSettingsJSON() const {
  base::Value::Dict dict;
  auto launch_params(
      base::JSONReader::Read(application_delegate_->GetLaunchParams()));
  if (launch_params)
    dict.Set(messages::kLaunchParams, std::move(launch_params).value());

  auto country(base::JSONReader::Read(system_delegate_->GetCountry()));
  if (country)
    dict.Set(messages::kCountry, std::move(country).value());

  dict.Set(messages::kLocale, system_delegate_->GetLocale());
  dict.Set(messages::kLocaleRegion, system_delegate_->GetLocaleRegion());
  dict.Set(messages::kIsMinimal,
           system_delegate_->IsMinimal() ? "true" : "false");
  dict.Set(messages::kIdentifier, application_delegate_->GetIdentifier());
  dict.Set(messages::kDevicePixelRatio, system_delegate_->GetDeviceInfoJSON());
  dict.Set(messages::kDevicePixelRatio, std::to_string(GetDevicePixelRatio()));
  dict.Set(messages::kTrustLevel, application_delegate_->GetTrustLevel());
  std::string json;
  base::JSONWriter::Write(dict, &json);
  return json;
}

int PlatformSystemHandler::AddExtension(
    PlatformSystemHandlerExtension* extension) {
  extensions_[extension_id_] = extension;
  return extension_id_++;
}

void PlatformSystemHandler::RemoveExtension(int id) {
  extensions_.erase(id);
}

std::string PlatformSystemHandler::Handle(
    const std::string& msg, const std::vector<std::string>& args) {
  if (messages::unhandled.count(msg)) {
    NOTIMPLEMENTED() << "msg handler for \"" << msg
                     << "\" is not implemented yet";
    return std::string();
  }

  // Web application related messages
  if (msg == messages::kIdentifier)
    return application_delegate_->GetIdentifier();

  if (msg == messages::kLaunchParams)
    return application_delegate_->GetLaunchParams();

  if (msg == messages::kFolderPath)
    return application_delegate_->GetFolderPath();

  if (msg == messages::kTrustLevel)
    return application_delegate_->GetTrustLevel();

  // System releated messages
  if (msg == messages::kCountry)
    return system_delegate_->GetCountry();

  if (msg == messages::kDeviceInfo)
    return system_delegate_->GetDeviceInfoJSON();

  if (msg == messages::kDevicePixelRatio) {
    return std::to_string(GetDevicePixelRatio());
  }

  if (msg == messages::kGetResource) {
    if (args.size() == 1)
      return system_delegate_->GetResource(args[0]);
  }

  if (msg == messages::kIsMinimal)
    return system_delegate_->IsMinimal() ? "true" : "false";

  if (msg == messages::kLocale)
    return system_delegate_->GetLocale();

  if (msg == messages::kLocaleRegion)
    return system_delegate_->GetLocaleRegion();

  // Gather information of system and application together
  if (msg == messages::kInitialize)
    return GetSettingsJSON();

  std::string result;
  for (auto& extension : extensions_) {
    if (extension.second->Handle(msg, args, result))
      return result;
  }

  return std::string();
}

void PlatformSystemHandler::OnSystemLanguageChanged() {
  NOTIMPLEMENTED();
}

float PlatformSystemHandler::GetDevicePixelRatio() const {
  NOTIMPLEMENTED();
  return 1.f;
}

}  // namespace pal
