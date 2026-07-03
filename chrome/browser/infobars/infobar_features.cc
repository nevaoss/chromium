// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/infobars/infobar_features.h"

#include <algorithm>
#include <vector>

#include "base/strings/string_split.h"
#include "base/strings/string_util.h"

namespace infobars {

constexpr char kDefaultMigratedInfoBars[] = "";

BASE_FEATURE(kCentralizedInfoBarFramework, base::FEATURE_DISABLED_BY_DEFAULT);

BASE_FEATURE_PARAM(bool, kEnableAll, &kCentralizedInfoBarFramework, false);

BASE_FEATURE_PARAM(std::string,
                   kMigrated,
                   &kCentralizedInfoBarFramework,
                   kDefaultMigratedInfoBars);

bool IsInfoBarMigrated(std::string_view infobar_id) {
  if (!base::FeatureList::IsEnabled(kCentralizedInfoBarFramework)) {
    return false;
  }
  if (kEnableAll.Get()) {
    return true;
  }
  std::string migrated_list = kMigrated.Get();
  std::vector<std::string_view> enabled_infobars = base::SplitStringPiece(
      migrated_list, ",", base::TRIM_WHITESPACE, base::SPLIT_WANT_NONEMPTY);
  return std::find(enabled_infobars.begin(), enabled_infobars.end(),
                   infobar_id) != enabled_infobars.end();
}

}  // namespace infobars
