// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_AUTOFILL_CORE_BROWSER_AT_MEMORY_AT_MEMORY_ENABLEMENT_UTILS_H_
#define COMPONENTS_AUTOFILL_CORE_BROWSER_AT_MEMORY_AT_MEMORY_ENABLEMENT_UTILS_H_

class PrefService;

namespace personal_context {
class PersonalContextEnablementService;
}  // namespace personal_context

namespace autofill {

enum class AtMemoryInvocationCustomizationSettingVisibility {
  kInvisible,
  kVisibleGreyedOut,
  kVisibleInteractable,
};

// Returns true if AtMemory is enabled.
//
// Checks that AtMemory feature flags are enabled, At-Memory eligibility
// criteria are met and PersonalContext settings toggle is on.
bool IsAtMemoryEnabled(personal_context::PersonalContextEnablementService*
                           personal_context_service,
                       PrefService* pref_service);

// Returns visibility of the AtMemory invocation customization setting in
// Autofill Settings.
//
// Checks the AtMemory feature flags, At-Memory eligibility criteria and the
// PersonalContext state to determine the
// `AtMemoryInvocationCustomizationSettingVisibility`.
AtMemoryInvocationCustomizationSettingVisibility
GetAtMemoryInvocationCustomizationSettingVisibility(
    personal_context::PersonalContextEnablementService*
        personal_context_service,
    PrefService* pref_service);

}  // namespace autofill

#endif  // COMPONENTS_AUTOFILL_CORE_BROWSER_AT_MEMORY_AT_MEMORY_ENABLEMENT_UTILS_H_
