// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/autofill/core/browser/at_memory/at_memory_enablement_utils.h"

#include <memory>
#include <optional>

#include "base/check_deref.h"
#include "base/test/scoped_feature_list.h"
#include "build/branding_buildflags.h"
#include "components/autofill/core/common/autofill_debug_features.h"
#include "components/autofill/core/common/autofill_features.h"
#include "components/personal_context/core/mock_personal_context_enablement_service.h"
#include "components/personal_context/core/personal_context_prefs.h"
#include "components/personal_context/core/personal_context_types.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/prefs/testing_pref_service.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

#if !BUILDFLAG(IS_FUCHSIA)
#include "components/variations/pref_names.h"                     // nogncheck
#include "components/variations/service/google_groups_manager.h"  // nogncheck
#include "components/variations/service/google_groups_manager_prefs.h"  // nogncheck
#include "components/variations/variations_seed_processor.h"  // nogncheck
#endif

namespace autofill {

namespace {

using ::testing::Return;

class AtMemoryEnablementUtilsTest : public testing::Test {
 protected:
  AtMemoryEnablementUtilsTest() {
    personal_context::prefs::RegisterProfilePrefs(pref_service_.registry());
    // Enable the toggle by default in tests since it represents the default
    // active state.
    pref_service_.SetUserPref(
        personal_context::prefs::kPersonalContextInAutofillSettingsToggleStatus,
        base::Value(true));
    // Set PersonalContextService to return not eligible by default.
    ON_CALL(personal_context_service_, GetEnablementState)
        .WillByDefault(Return(personal_context::PersonalContextEnablementState::
                                  kDisabledNotEligible));
  }

  base::test::ScopedFeatureList feature_list_{features::kAutofillAtMemory};
  testing::NiceMock<personal_context::MockPersonalContextEnablementService>
      personal_context_service_;
  TestingPrefServiceSimple pref_service_;
};

#if BUILDFLAG(GOOGLE_CHROME_BRANDING)

// Tests that `MayPerformAtMemoryAction` returns false when AtMemory is
// disabled.
TEST_F(AtMemoryEnablementUtilsTest, MayPerformAtMemoryAction_AtMemoryDisabled) {
  base::test::ScopedFeatureList disabled_features;
  disabled_features.InitAndDisableFeature(features::kAutofillAtMemory);
  EXPECT_CALL(personal_context_service_, GetEnablementState)
      .WillRepeatedly(
          Return(personal_context::PersonalContextEnablementState::kEnabled));

  EXPECT_FALSE(MayPerformAtMemoryAction(AtMemoryAction::kTriggerSearchUI,
                                        &personal_context_service_,
                                        &pref_service_, nullptr));
  EXPECT_FALSE(MayPerformAtMemoryAction(AtMemoryAction::kShowAtMemoryInSettings,
                                        &personal_context_service_,
                                        &pref_service_, nullptr));
  EXPECT_FALSE(MayPerformAtMemoryAction(
      AtMemoryAction::kAllowCustomizeAtMemoryShortcut,
      &personal_context_service_, &pref_service_, nullptr));
}

// Tests that `MayPerformAtMemoryAction` returns false when
// `personal_context_service` is null.
TEST_F(AtMemoryEnablementUtilsTest,
       MayPerformAtMemoryAction_NullPersonalContextService) {
  EXPECT_FALSE(MayPerformAtMemoryAction(AtMemoryAction::kTriggerSearchUI,
                                        nullptr, &pref_service_, nullptr));
  EXPECT_FALSE(MayPerformAtMemoryAction(AtMemoryAction::kShowAtMemoryInSettings,
                                        nullptr, &pref_service_, nullptr));
  EXPECT_FALSE(
      MayPerformAtMemoryAction(AtMemoryAction::kAllowCustomizeAtMemoryShortcut,
                               nullptr, &pref_service_, nullptr));
}

// Tests `MayPerformAtMemoryAction` when `pref_service` is null.
TEST_F(AtMemoryEnablementUtilsTest, MayPerformAtMemoryAction_NullPrefService) {
  EXPECT_CALL(personal_context_service_, GetEnablementState)
      .WillRepeatedly(
          Return(personal_context::PersonalContextEnablementState::kEnabled));

  EXPECT_FALSE(MayPerformAtMemoryAction(AtMemoryAction::kTriggerSearchUI,
                                        &personal_context_service_, nullptr,
                                        nullptr));
  EXPECT_TRUE(MayPerformAtMemoryAction(AtMemoryAction::kShowAtMemoryInSettings,
                                       &personal_context_service_, nullptr,
                                       nullptr));
  EXPECT_FALSE(
      MayPerformAtMemoryAction(AtMemoryAction::kAllowCustomizeAtMemoryShortcut,
                               &personal_context_service_, nullptr, nullptr));
}

// Tests `MayPerformAtMemoryAction` under various Personal Context states.
TEST_F(AtMemoryEnablementUtilsTest, MayPerformAtMemoryAction_States) {
  pref_service_.SetUserPref(
      personal_context::prefs::kPersonalContextInAutofillSettingsToggleStatus,
      base::Value(true));

  // State: kEnabled
  EXPECT_CALL(personal_context_service_, GetEnablementState)
      .WillOnce(
          Return(personal_context::PersonalContextEnablementState::kEnabled));
  EXPECT_TRUE(MayPerformAtMemoryAction(
      AtMemoryAction::kAllowCustomizeAtMemoryShortcut,
      &personal_context_service_, &pref_service_, nullptr));

  // State: kEnabledShouldShowNotice
  EXPECT_CALL(personal_context_service_, GetEnablementState)
      .WillOnce(Return(personal_context::PersonalContextEnablementState::
                           kEnabledShouldShowNotice));
  pref_service_.SetUserPref(
      personal_context::prefs::kPersonalContextInAutofillSettingsToggleStatus,
      base::Value(true));
  EXPECT_TRUE(MayPerformAtMemoryAction(
      AtMemoryAction::kAllowCustomizeAtMemoryShortcut,
      &personal_context_service_, &pref_service_, nullptr));

  // State: kDisabledViaPersonalIntelligenceInAutofillToggle
  EXPECT_CALL(personal_context_service_, GetEnablementState)
      .WillOnce(Return(personal_context::PersonalContextEnablementState::
                           kDisabledViaPersonalIntelligenceInAutofillToggle));
  pref_service_.SetUserPref(
      personal_context::prefs::kPersonalContextInAutofillSettingsToggleStatus,
      base::Value(false));
  EXPECT_TRUE(MayPerformAtMemoryAction(AtMemoryAction::kShowAtMemoryInSettings,
                                       &personal_context_service_,
                                       &pref_service_, nullptr));

  // State: kDisabledNeedsOptIn
  EXPECT_CALL(personal_context_service_, GetEnablementState)
      .WillOnce(Return(personal_context::PersonalContextEnablementState::
                           kDisabledNeedsOptIn));
  pref_service_.SetUserPref(
      personal_context::prefs::kPersonalContextInAutofillSettingsToggleStatus,
      base::Value(false));
  EXPECT_TRUE(MayPerformAtMemoryAction(AtMemoryAction::kShowAtMemoryInSettings,
                                       &personal_context_service_,
                                       &pref_service_, nullptr));

  // State: kDisabledNotEligible
  EXPECT_CALL(personal_context_service_, GetEnablementState)
      .WillOnce(Return(personal_context::PersonalContextEnablementState::
                           kDisabledNotEligible));
  EXPECT_FALSE(MayPerformAtMemoryAction(AtMemoryAction::kTriggerSearchUI,
                                        &personal_context_service_,
                                        &pref_service_, nullptr));
}

// Tests `MayPerformAtMemoryAction` when the toggle pref is off.
TEST_F(AtMemoryEnablementUtilsTest, MayPerformAtMemoryAction_ToggleOff) {
  EXPECT_CALL(personal_context_service_, GetEnablementState)
      .WillRepeatedly(
          Return(personal_context::PersonalContextEnablementState::kEnabled));
  pref_service_.SetUserPref(
      personal_context::prefs::kPersonalContextInAutofillSettingsToggleStatus,
      base::Value(false));

  EXPECT_FALSE(MayPerformAtMemoryAction(AtMemoryAction::kTriggerSearchUI,
                                        &personal_context_service_,
                                        &pref_service_, nullptr));
  EXPECT_TRUE(MayPerformAtMemoryAction(AtMemoryAction::kShowAtMemoryInSettings,
                                       &personal_context_service_,
                                       &pref_service_, nullptr));
  EXPECT_FALSE(MayPerformAtMemoryAction(
      AtMemoryAction::kAllowCustomizeAtMemoryShortcut,
      &personal_context_service_, &pref_service_, nullptr));
}

// Tests that `MayPerformAtMemoryAction returns false when
// `personal_context_service` returns `kDisabledNotEligible`.
TEST_F(AtMemoryEnablementUtilsTest, MayPerformAtMemoryAction_NotSupported) {
  EXPECT_CALL(personal_context_service_, GetEnablementState)
      .WillRepeatedly(Return(personal_context::PersonalContextEnablementState::
                                 kDisabledNotEligible));
  EXPECT_FALSE(MayPerformAtMemoryAction(AtMemoryAction::kTriggerSearchUI,
                                        &personal_context_service_,
                                        &pref_service_, nullptr));
}

// Tests that `MayPerformAtMemoryAction` returns true when the client supports
// AtMemory and the settings toggle is enabled.
TEST_F(AtMemoryEnablementUtilsTest,
       MayPerformAtMemoryAction_SupportedAndToggleOn) {
  EXPECT_CALL(personal_context_service_, GetEnablementState)
      .WillRepeatedly(
          Return(personal_context::PersonalContextEnablementState::kEnabled));
  pref_service_.SetUserPref(
      personal_context::prefs::kPersonalContextInAutofillSettingsToggleStatus,
      base::Value(true));
  EXPECT_TRUE(MayPerformAtMemoryAction(AtMemoryAction::kTriggerSearchUI,
                                       &personal_context_service_,
                                       &pref_service_, nullptr));
}

// Tests that when `kAtMemorySkipEligibilityChecks` is enabled,
// `MayPerformAtMemoryAction` returns true even if the user is not eligible,
// provided that the base `kAutofillAtMemory` feature is enabled.
TEST_F(AtMemoryEnablementUtilsTest,
       MayPerformAtMemoryAction_SkipEligibilityChecks) {
  base::test::ScopedFeatureList debug_features(
      features::debug::kAtMemorySkipEligibilityChecks);

  EXPECT_CALL(personal_context_service_, GetEnablementState)
      .WillRepeatedly(Return(personal_context::PersonalContextEnablementState::
                                 kDisabledNotEligible));

  EXPECT_TRUE(MayPerformAtMemoryAction(AtMemoryAction::kTriggerSearchUI,
                                       &personal_context_service_,
                                       &pref_service_, nullptr));
}

#endif  // BUILDFLAG(GOOGLE_CHROME_BRANDING)

// Tests for non-branded Chromium builds.
#if !BUILDFLAG(GOOGLE_CHROME_BRANDING)
// Tests that MayPerformAtMemoryAction returns false for non-branded Chromium
// build even when all conditions are met.
TEST_F(AtMemoryEnablementUtilsTest,
       MayPerformAtMemoryAction_SupportedAndToggleOn) {
  EXPECT_CALL(personal_context_service_, GetEnablementState)
      .WillRepeatedly(
          Return(personal_context::PersonalContextEnablementState::kEnabled));
  pref_service_.SetUserPref(
      personal_context::prefs::kPersonalContextInAutofillSettingsToggleStatus,
      base::Value(true));
  EXPECT_FALSE(MayPerformAtMemoryAction(AtMemoryAction::kTriggerSearchUI,
                                        &personal_context_service_,
                                        &pref_service_, nullptr));
}
#endif  // !BUILDFLAG(GOOGLE_CHROME_BRANDING)

#if BUILDFLAG(GOOGLE_CHROME_BRANDING) && !BUILDFLAG(IS_FUCHSIA)
class AtMemoryEnablementUtilsWithGroupsTest
    : public AtMemoryEnablementUtilsTest {
 protected:
  AtMemoryEnablementUtilsWithGroupsTest() {
    local_state_.registry()->RegisterDictionaryPref(
        variations::prefs::kVariationsGoogleGroups);
    pref_service_.registry()->RegisterListPref(GetDogfoodGroupsPrefName());

    groups_manager_.emplace(local_state_, "DefaultKey", pref_service_);
  }

  static constexpr std::string GetDogfoodGroupsPrefName() {
#if BUILDFLAG(IS_CHROMEOS)
    return variations::kOsDogfoodGroupsSyncPrefName;
#else
    return variations::kDogfoodGroupsSyncPrefName;
#endif
  }

  void SetUserGroups(const std::vector<std::string>& groups) {
    base::ListValue pref_groups_list;
    for (const std::string& group : groups) {
      base::DictValue group_dict;
      group_dict.Set(variations::kDogfoodGroupsSyncPrefGaiaIdKey, group);
      pref_groups_list.Append(std::move(group_dict));
    }
    pref_service_.SetUserPref(GetDogfoodGroupsPrefName(),
                              base::Value(std::move(pref_groups_list)));
  }

  const GoogleGroupsManager& groups_manager() const {
    return CHECK_DEREF(groups_manager_);
  }

 private:
  TestingPrefServiceSimple local_state_;
  std::optional<GoogleGroupsManager> groups_manager_;
};

// Tests that the action is not allowed if a Google Group is required but the
// user is not a member of that group.
TEST_F(AtMemoryEnablementUtilsWithGroupsTest,
       MayPerformAtMemoryAction_GroupRequired_UserNotInGroup) {
  constexpr char kRequiredGroup[] = "at-memory-dogfooders";
  base::test::ScopedFeatureList local_feature_list;
  local_feature_list.InitAndEnableFeatureWithParameters(
      features::kAutofillAtMemory,
      {{variations::internal::kGoogleGroupFeatureParamName, kRequiredGroup}});

  SetUserGroups({"some-other-group", "another-group"});
  ON_CALL(personal_context_service_, GetEnablementState)
      .WillByDefault(
          Return(personal_context::PersonalContextEnablementState::kEnabled));

  EXPECT_FALSE(MayPerformAtMemoryAction(AtMemoryAction::kTriggerSearchUI,
                                        &personal_context_service_,
                                        &pref_service_, &groups_manager()));
}

// Tests that the action is allowed if a Google Group is required and the user
// is a member of that group.
TEST_F(AtMemoryEnablementUtilsWithGroupsTest,
       MayPerformAtMemoryAction_GroupRequired_UserInGroup) {
  constexpr char kRequiredGroup[] = "at-memory-dogfooders";
  base::test::ScopedFeatureList local_feature_list;
  local_feature_list.InitAndEnableFeatureWithParameters(
      features::kAutofillAtMemory,
      {{variations::internal::kGoogleGroupFeatureParamName, kRequiredGroup}});

  SetUserGroups({"some-other-group", kRequiredGroup});
  ON_CALL(personal_context_service_, GetEnablementState)
      .WillByDefault(
          Return(personal_context::PersonalContextEnablementState::kEnabled));

  EXPECT_TRUE(MayPerformAtMemoryAction(AtMemoryAction::kTriggerSearchUI,
                                       &personal_context_service_,
                                       &pref_service_, &groups_manager()));
}
#endif  // BUILDFLAG(GOOGLE_CHROME_BRANDING) && !BUILDFLAG(IS_FUCHSIA)

}  // namespace
}  // namespace autofill
