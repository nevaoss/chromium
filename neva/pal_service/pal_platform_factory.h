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

#ifndef NEVA_PAL_SERVICE_PLATFORM_FACTORY_H_
#define NEVA_PAL_SERVICE_PLATFORM_FACTORY_H_

#include <memory>
#include <string>

#include "base/component_export.h"
#include "neva/injection/public/common/webapi_names.h"
#include "neva/pal_service/public/application_registrator_delegate.h"
#include "neva/pal_service/public/language_tracker_delegate.h"
#include "neva/pal_service/public/notification_manager_delegate.h"
#include "neva/pal_service/public/system_servicebridge_delegate.h"

namespace pal {
class ExternalProtocolHandlerDelegate;
class MemoryManagerDelegate;
class NetworkErrorPageControllerDelegate;
class PlatformSystemDelegate;

class COMPONENT_EXPORT(PAL_SERVICE) PlatformFactory {
 public:
  static PlatformFactory* Get();

  // Check whether an injection can be created for a particular platform.
  bool CanCreateInjection(std::string_view name);

  std::unique_ptr<ApplicationRegistratorDelegate>
  CreateApplicationRegistratorDelegate(
      std::string application_name,
      ApplicationRegistratorDelegate::RepeatingResponse callback);

  std::unique_ptr<LanguageTrackerDelegate> CreateLanguageTrackerDelegate(
      LanguageTrackerDelegate::RepeatingResponse callback);

  std::unique_ptr<MemoryManagerDelegate> CreateMemoryManagerDelegate();

  std::unique_ptr<SystemServiceBridgeDelegate>
  CreateSystemServiceBridgeDelegate(
      SystemServiceBridgeDelegate::CreationParams params,
      SystemServiceBridgeDelegate::Response callback);

  std::unique_ptr<PlatformSystemDelegate> CreatePlatformSystemDelegate();

  std::unique_ptr<NetworkErrorPageControllerDelegate>
  CreateNetworkErrorPageControllerDelegate();

  std::unique_ptr<NotificationManagerDelegate>
  CreateNotificationManagerDelegate();

  std::unique_ptr<ExternalProtocolHandlerDelegate>
  CreateExternalProtocolHandlerDelegate();

 private:
  PlatformFactory();
};

}  // namespace pal

#endif  // NEVA_PAL_SERVICE_PLATFORM_FACTORY_H_
