// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/autofill/core/browser/integrators/personal_context/personal_context_autofill_util.h"

#include "base/feature_list.h"
#include "components/autofill/core/browser/at_memory/at_memory_enablement_utils.h"
#include "components/autofill/core/common/autofill_features.h"
#include "components/personal_context/core/personal_context_enablement_service.h"
#include "components/personal_context/core/personal_context_types.h"
#include "components/prefs/pref_service.h"

namespace autofill {

bool ShouldShowPersonalContextAutofillSetting(
    personal_context::PersonalContextEnablementService* enablement_service,
    const subscription_eligibility::SubscriptionEligibilityService*
        subscription_eligibility_service,
    const PrefService* pref_service,
    const GoogleGroupsManager* google_groups_manager) {
  if (!enablement_service) {
    return false;
  }
  // TODO(crbug.com/523168644) Replace the simple feature check with a call to
  // `MayPerformAutofillAiAction(kAmbientAutofill)`.
  const bool ambient_autofill_enabled =
      base::FeatureList::IsEnabled(features::kAutofillAmbientAutofill);
  const bool at_memory_enabled = MayPerformAtMemoryAction(
      AtMemoryAction::kShowAtMemoryInSettings, enablement_service,
      subscription_eligibility_service, pref_service, google_groups_manager);
  if (!ambient_autofill_enabled && !at_memory_enabled) {
    return false;
  }

  using enum personal_context::PersonalContextEnablementState;
  switch (enablement_service->GetEnablementState()) {
    case kDisabledNotEligible:
    case kDisabledNeedsOptIn:
      return false;
    case kEnabledShouldShowNotice:
    case kDisabledViaPersonalIntelligenceInAutofillToggle:
    case kEnabled:
      return true;
  }
}

}  // namespace autofill
