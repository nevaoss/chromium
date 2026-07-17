// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/themes/cross_device/cross_device_theme_tracker_desktop.h"

#include "base/check.h"
#include "base/functional/bind.h"
#include "components/sync/base/report_unrecoverable_error.h"
#include "components/sync/model/client_tag_based_data_type_processor.h"
#include "components/sync/model/data_type_store_service.h"

namespace themes {

namespace {

sync_pb::ThemeSpecifics TranslateAndroidTheme(
    const sync_pb::ThemeAndroidSpecifics& android_specifics) {
  sync_pb::ThemeSpecifics desktop_specifics;
  if (android_specifics.has_use_custom_theme()) {
    desktop_specifics.set_use_custom_theme(
        android_specifics.use_custom_theme());
  }
  if (android_specifics.has_ntp_background()) {
    *desktop_specifics.mutable_ntp_background() =
        android_specifics.ntp_background();
  }
  if (android_specifics.has_user_color_theme()) {
    *desktop_specifics.mutable_user_color_theme() =
        android_specifics.user_color_theme();
  }
  return desktop_specifics;
}

sync_pb::ThemeSpecifics TranslateIosTheme(
    const sync_pb::ThemeIosSpecifics& ios_specifics) {
  sync_pb::ThemeSpecifics desktop_specifics;
  if (ios_specifics.has_user_color_theme()) {
    *desktop_specifics.mutable_user_color_theme() =
        ios_specifics.user_color_theme();
  }
  if (ios_specifics.has_ntp_background()) {
    *desktop_specifics.mutable_ntp_background() =
        ios_specifics.ntp_background();
  }
  return desktop_specifics;
}

}  // namespace

DeviceThemeInfo<sync_pb::ThemeSpecifics> TranslateAndroid(
    const sync_pb::ThemeAndroidSpecifics& android_specifics) {
  DeviceThemeInfo<sync_pb::ThemeSpecifics> info;
  info.os_type = syncer::DeviceInfo::OsType::kAndroid;
  info.theme = TranslateAndroidTheme(android_specifics);
  return info;
}

DeviceThemeInfo<sync_pb::ThemeSpecifics> TranslateIos(
    const sync_pb::ThemeIosSpecifics& ios_specifics) {
  DeviceThemeInfo<sync_pb::ThemeSpecifics> info;
  info.os_type = syncer::DeviceInfo::OsType::kIOS;
  info.theme = TranslateIosTheme(ios_specifics);
  return info;
}

CrossDeviceThemeTrackerDesktop::CrossDeviceThemeTrackerDesktop(
    syncer::DeviceInfoTracker* device_info_tracker,
    AndroidBridgeFactory android_bridge_factory,
    IosBridgeFactory ios_bridge_factory)
    : CrossDeviceThemeTracker<sync_pb::ThemeSpecifics>(device_info_tracker) {
  android_bridge_ = std::move(android_bridge_factory).Run(this);
  ios_bridge_ = std::move(ios_bridge_factory).Run(this);
  CHECK(android_bridge_);
  CHECK(ios_bridge_);
}

CrossDeviceThemeTrackerDesktop::~CrossDeviceThemeTrackerDesktop() = default;

base::WeakPtr<syncer::DataTypeControllerDelegate>
CrossDeviceThemeTrackerDesktop::GetAndroidSyncDelegate() {
  CHECK(android_bridge_);
  return android_bridge_->change_processor()->GetControllerDelegate();
}

base::WeakPtr<syncer::DataTypeControllerDelegate>
CrossDeviceThemeTrackerDesktop::GetIosSyncDelegate() {
  CHECK(ios_bridge_);
  return ios_bridge_->change_processor()->GetControllerDelegate();
}

}  // namespace themes
