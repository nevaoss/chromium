// Copyright 2021 LG Electronics, Inc.
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

#include "neva/pal_service/pal_platform_factory.h"

#include <memory>

#include "neva/pal_service/dummy/platform_system_delegate_dummy.h"
#include "neva/pal_service/dummy/system_servicebridge_delegate_dummy.h"
#include "neva/pal_service/public/application_registrator_delegate.h"
#include "neva/pal_service/public/external_protocol_handler_delegate.h"
#include "neva/pal_service/public/language_tracker_delegate.h"
#include "neva/pal_service/public/memorymanager_delegate.h"
#include "neva/pal_service/public/network_error_page_controller_delegate.h"

namespace pal {

namespace {

std::set<std::string_view> allowed_injections = {
    injections::webapi::kBrowserControl,
    injections::webapi::kNetworkErrorPage,
    injections::webapi::kSample,
    injections::webapi::kWebOSServiceBridge,
    injections::webapi::kWebOSServiceBridgeObsolete,
    injections::webapi::kWebOSSystem,
    injections::webapi::kWebOSSystemObsolete,
};

}  // namespace

bool PlatformFactory::CanCreateInjection(std::string_view name) {
  return allowed_injections.count(name) != 0;
}

std::unique_ptr<ApplicationRegistratorDelegate>
PlatformFactory::CreateApplicationRegistratorDelegate(
    std::string application_name,
    ApplicationRegistratorDelegate::RepeatingResponse callback) {
  return std::unique_ptr<ApplicationRegistratorDelegate>();
}

std::unique_ptr<LanguageTrackerDelegate>
PlatformFactory::CreateLanguageTrackerDelegate(
    LanguageTrackerDelegate::RepeatingResponse callback) {
  return std::unique_ptr<LanguageTrackerDelegate>();
}

std::unique_ptr<MemoryManagerDelegate>
PlatformFactory::CreateMemoryManagerDelegate() {
  return std::unique_ptr<MemoryManagerDelegate>();
}

std::unique_ptr<SystemServiceBridgeDelegate>
PlatformFactory::CreateSystemServiceBridgeDelegate(
    SystemServiceBridgeDelegate::CreationParams params,
    SystemServiceBridgeDelegate::Response callback) {
  // When the implementation for AGL is done next lines should be
  // replaced with
  // return std::make_unique<agl::SystemServiceBridgeDelegateAGL>(
  //     std::move(params), std::move(callback));
  return std::make_unique<dummy::SystemServiceBridgeDelegateDummy>(
      std::move(params), std::move(callback));
}

std::unique_ptr<PlatformSystemDelegate>
PlatformFactory::CreatePlatformSystemDelegate() {
  // PlatformSystemDelegate is used to provide a browser side handler of
  // WebOSSystemInjection requests. If such implementation exists for AGL
  // next lines should be replaced with
  // return std::make_unique<agl::PlatformSystemDelegateAGL>();
  return std::make_unique<dummy::PlatformSystemDelegateDummy>();
}

std::unique_ptr<NetworkErrorPageControllerDelegate>
PlatformFactory::CreateNetworkErrorPageControllerDelegate() {
  return std::unique_ptr<NetworkErrorPageControllerDelegate>();
}

std::unique_ptr<NotificationManagerDelegate>
PlatformFactory::CreateNotificationManagerDelegate() {
  return std::unique_ptr<NotificationManagerDelegate>();
}

std::unique_ptr<ExternalProtocolHandlerDelegate>
PlatformFactory::CreateExternalProtocolHandlerDelegate() {
  return std::unique_ptr<ExternalProtocolHandlerDelegate>();
}

}  // namespace pal
