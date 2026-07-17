// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_AUTOFILL_CORE_BROWSER_INTEGRATORS_PERSONAL_CONTEXT_PERSONAL_CONTEXT_AUTOFILL_UTIL_H_
#define COMPONENTS_AUTOFILL_CORE_BROWSER_INTEGRATORS_PERSONAL_CONTEXT_PERSONAL_CONTEXT_AUTOFILL_UTIL_H_

#include "components/personal_context/core/personal_context_types.h"

namespace personal_context {
class PersonalContextEnablementService;
}

class GoogleGroupsManager;
class PrefService;

namespace subscription_eligibility {
class SubscriptionEligibilityService;
}

namespace autofill {

// Returns true if the Personal Context setting should be shown in the
// Autofill settings page.
bool ShouldShowPersonalContextAutofillSetting(
    personal_context::PersonalContextEnablementService* enablement_service,
    const subscription_eligibility::SubscriptionEligibilityService*
        subscription_eligibility_service,
    const PrefService* pref_service,
    const GoogleGroupsManager* google_groups_manager);

}  // namespace autofill

#endif  // COMPONENTS_AUTOFILL_CORE_BROWSER_INTEGRATORS_PERSONAL_CONTEXT_PERSONAL_CONTEXT_AUTOFILL_UTIL_H_
