// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_INFOBARS_INFOBAR_FEATURES_H_
#define CHROME_BROWSER_INFOBARS_INFOBAR_FEATURES_H_

#include <string>
#include <string_view>

#include "base/feature_list.h"
#include "base/metrics/field_trial_params.h"

namespace infobars {

// Feature flag controlling the centralization of desktop infobars.
// TODO(https://crbug.com/512837934): Remove feature flag once fully launched
// and all feature-specific delegates are migrated.
BASE_DECLARE_FEATURE(kCentralizedInfoBarFramework);

BASE_DECLARE_FEATURE_PARAM(bool, kEnableAll);
BASE_DECLARE_FEATURE_PARAM(std::string, kMigrated);

// Returns true if the centralization framework is enabled and the specified
// infobar is configured to be migrated (either via "enable_all" or listed in
// "migrated_infobars").
bool IsInfoBarMigrated(std::string_view infobar_id);

}  // namespace infobars

#endif  // CHROME_BROWSER_INFOBARS_INFOBAR_FEATURES_H_
