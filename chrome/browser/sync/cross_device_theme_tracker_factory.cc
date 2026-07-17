// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/sync/cross_device_theme_tracker_factory.h"

#include "base/functional/bind.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/profiles/profile_selections.h"
#include "chrome/browser/sync/data_type_store_service_factory.h"
#include "chrome/browser/sync/device_info_sync_service_factory.h"
#include "chrome/browser/themes/cross_device/cross_device_theme_tracker_desktop.h"
#include "chrome/common/channel_info.h"
#include "components/sync/base/report_unrecoverable_error.h"
#include "components/sync/model/client_tag_based_data_type_processor.h"
#include "components/sync/model/data_type_store_service.h"
#include "components/sync_device_info/device_info_sync_service.h"
#include "components/themes/cross_device/cross_device_theme_sync_bridge.h"

// static
themes::CrossDeviceThemeTrackerDesktop*
CrossDeviceThemeTrackerFactory::GetForProfile(Profile* profile) {
  return static_cast<themes::CrossDeviceThemeTrackerDesktop*>(
      GetInstance()->GetServiceForBrowserContext(profile, true));
}

// static
CrossDeviceThemeTrackerFactory* CrossDeviceThemeTrackerFactory::GetInstance() {
  static base::NoDestructor<CrossDeviceThemeTrackerFactory> instance;
  return instance.get();
}

CrossDeviceThemeTrackerFactory::CrossDeviceThemeTrackerFactory()
    : ProfileKeyedServiceFactory(
          "CrossDeviceThemeTracker",
          ProfileSelections::Builder()
              .WithRegular(ProfileSelection::kOriginalOnly)
              .WithGuest(ProfileSelection::kOriginalOnly)
              .Build()) {
  DependsOn(DeviceInfoSyncServiceFactory::GetInstance());
  DependsOn(DataTypeStoreServiceFactory::GetInstance());
}

CrossDeviceThemeTrackerFactory::~CrossDeviceThemeTrackerFactory() = default;

std::unique_ptr<KeyedService>
CrossDeviceThemeTrackerFactory::BuildServiceInstanceForBrowserContext(
    content::BrowserContext* context) const {
  Profile* profile = Profile::FromBrowserContext(context);

  syncer::DeviceInfoSyncService* device_info_sync_service =
      DeviceInfoSyncServiceFactory::GetForProfile(profile);
  syncer::DeviceInfoTracker* device_info_tracker =
      device_info_sync_service
          ? device_info_sync_service->GetDeviceInfoTracker()
          : nullptr;

  syncer::DataTypeStoreService* store_service =
      DataTypeStoreServiceFactory::GetForProfile(profile);
  version_info::Channel channel = chrome::GetChannel();

  auto android_bridge_factory = base::BindOnce(
      [](syncer::DataTypeStoreService* store_service,
         version_info::Channel channel,
         themes::CrossDeviceThemeTrackerDesktop* tracker)
          -> std::unique_ptr<
              themes::CrossDeviceThemeTrackerDesktop::AndroidBridge> {
        syncer::RepeatingDataTypeStoreFactory store_factory =
            store_service->GetStoreFactory();
        auto android_processor =
            std::make_unique<syncer::ClientTagBasedDataTypeProcessor>(
                syncer::THEMES_ANDROID,
                base::BindRepeating(&syncer::ReportUnrecoverableError,
                                    channel));
        return std::make_unique<themes::CrossDeviceThemeSyncBridge<
            sync_pb::ThemeAndroidSpecifics, sync_pb::ThemeSpecifics>>(
            syncer::THEMES_ANDROID,
            base::BindRepeating(&themes::TranslateAndroid),
            base::BindRepeating(
                &themes::CrossDeviceThemeTrackerDesktop::UpdateThemeInfo,
                base::Unretained(tracker)),
            base::BindRepeating(
                &themes::CrossDeviceThemeTrackerDesktop::RemoveThemeInfo,
                base::Unretained(tracker)),
            std::move(android_processor), store_factory);
      },
      base::Unretained(store_service), channel);

  auto ios_bridge_factory = base::BindOnce(
      [](syncer::DataTypeStoreService* store_service,
         version_info::Channel channel,
         themes::CrossDeviceThemeTrackerDesktop* tracker)
          -> std::unique_ptr<
              themes::CrossDeviceThemeTrackerDesktop::IosBridge> {
        syncer::RepeatingDataTypeStoreFactory store_factory =
            store_service->GetStoreFactory();
        auto ios_processor =
            std::make_unique<syncer::ClientTagBasedDataTypeProcessor>(
                syncer::THEMES_IOS,
                base::BindRepeating(&syncer::ReportUnrecoverableError,
                                    channel));
        return std::make_unique<themes::CrossDeviceThemeSyncBridge<
            sync_pb::ThemeIosSpecifics, sync_pb::ThemeSpecifics>>(
            syncer::THEMES_IOS, base::BindRepeating(&themes::TranslateIos),
            base::BindRepeating(
                &themes::CrossDeviceThemeTrackerDesktop::UpdateThemeInfo,
                base::Unretained(tracker)),
            base::BindRepeating(
                &themes::CrossDeviceThemeTrackerDesktop::RemoveThemeInfo,
                base::Unretained(tracker)),
            std::move(ios_processor), store_factory);
      },
      base::Unretained(store_service), channel);

  return std::make_unique<themes::CrossDeviceThemeTrackerDesktop>(
      device_info_tracker, std::move(android_bridge_factory),
      std::move(ios_bridge_factory));
}
