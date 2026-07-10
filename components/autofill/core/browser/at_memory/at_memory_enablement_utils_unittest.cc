// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/autofill/core/browser/at_memory/at_memory_enablement_utils.h"

#include <memory>

#include "base/test/scoped_feature_list.h"
#include "components/autofill/core/common/autofill_features.h"
#include "components/personal_context/core/mock_personal_context_enablement_service.h"
#include "components/personal_context/core/personal_context_prefs.h"
#include "components/personal_context/core/personal_context_types.h"
#include "components/prefs/testing_pref_service.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

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

// Tests that setting visibility is kInvisible when At-Memory is disabled.
TEST_F(AtMemoryEnablementUtilsTest,
       InvocationCustomizationSettingVisibility_AtMemoryDisabled) {
  base::test::ScopedFeatureList disabled_features;
  disabled_features.InitAndDisableFeature(features::kAutofillAtMemory);
  EXPECT_CALL(personal_context_service_, GetEnablementState)
      .WillRepeatedly(
          Return(personal_context::PersonalContextEnablementState::kEnabled));
  EXPECT_EQ(GetAtMemoryInvocationCustomizationSettingVisibility(
                &personal_context_service_, &pref_service_),
            AtMemoryInvocationCustomizationSettingVisibility::kInvisible);
}

// Tests that setting visibility is kInvisible when personal_context_service is
// null.
TEST_F(AtMemoryEnablementUtilsTest,
       InvocationCustomizationSettingVisibility_NullPersonalContextService) {
  EXPECT_EQ(GetAtMemoryInvocationCustomizationSettingVisibility(nullptr,
                                                                &pref_service_),
            AtMemoryInvocationCustomizationSettingVisibility::kInvisible);
}

// Tests that setting visibility is kVisibleGreyedOut when pref_service is null.
TEST_F(AtMemoryEnablementUtilsTest,
       InvocationCustomizationSettingVisibility_NullPrefService) {
  EXPECT_CALL(personal_context_service_, GetEnablementState)
      .WillRepeatedly(
          Return(personal_context::PersonalContextEnablementState::kEnabled));
  EXPECT_EQ(
      GetAtMemoryInvocationCustomizationSettingVisibility(
          &personal_context_service_, nullptr),
      AtMemoryInvocationCustomizationSettingVisibility::kVisibleGreyedOut);
}

// Tests setting visibility under various PersonalContext states.
TEST_F(AtMemoryEnablementUtilsTest,
       InvocationCustomizationSettingVisibility_States) {
  pref_service_.SetUserPref(
      personal_context::prefs::kPersonalContextInAutofillSettingsToggleStatus,
      base::Value(true));

  // State: kEnabled -> kVisibleInteractable
  EXPECT_CALL(personal_context_service_, GetEnablementState)
      .WillOnce(
          Return(personal_context::PersonalContextEnablementState::kEnabled));
  EXPECT_EQ(
      GetAtMemoryInvocationCustomizationSettingVisibility(
          &personal_context_service_, &pref_service_),
      AtMemoryInvocationCustomizationSettingVisibility::kVisibleInteractable);

  // State: kEnabledShouldShowNotice -> kVisibleInteractable
  EXPECT_CALL(personal_context_service_, GetEnablementState)
      .WillOnce(Return(personal_context::PersonalContextEnablementState::
                           kEnabledShouldShowNotice));
  pref_service_.SetUserPref(
      personal_context::prefs::kPersonalContextInAutofillSettingsToggleStatus,
      base::Value(true));
  EXPECT_EQ(
      GetAtMemoryInvocationCustomizationSettingVisibility(
          &personal_context_service_, &pref_service_),
      AtMemoryInvocationCustomizationSettingVisibility::kVisibleInteractable);

  // State: kDisabledViaPersonalIntelligenceInAutofillToggle ->
  // kVisibleGreyedOut
  EXPECT_CALL(personal_context_service_, GetEnablementState)
      .WillOnce(Return(personal_context::PersonalContextEnablementState::
                           kDisabledViaPersonalIntelligenceInAutofillToggle));
  pref_service_.SetUserPref(
      personal_context::prefs::kPersonalContextInAutofillSettingsToggleStatus,
      base::Value(false));
  EXPECT_EQ(
      GetAtMemoryInvocationCustomizationSettingVisibility(
          &personal_context_service_, &pref_service_),
      AtMemoryInvocationCustomizationSettingVisibility::kVisibleGreyedOut);

  // State: kDisabledNeedsOptIn -> kVisibleGreyedOut
  // Note: the settings visibility is greyed out because the client supports the
  // feature, even though opt-in is needed.
  EXPECT_CALL(personal_context_service_, GetEnablementState)
      .WillOnce(Return(personal_context::PersonalContextEnablementState::
                           kDisabledNeedsOptIn));
  pref_service_.SetUserPref(
      personal_context::prefs::kPersonalContextInAutofillSettingsToggleStatus,
      base::Value(false));
  EXPECT_EQ(
      GetAtMemoryInvocationCustomizationSettingVisibility(
          &personal_context_service_, &pref_service_),
      AtMemoryInvocationCustomizationSettingVisibility::kVisibleGreyedOut);

  // State: kDisabledNotEligible -> kInvisible
  EXPECT_CALL(personal_context_service_, GetEnablementState)
      .WillOnce(Return(personal_context::PersonalContextEnablementState::
                           kDisabledNotEligible));
  EXPECT_EQ(GetAtMemoryInvocationCustomizationSettingVisibility(
                &personal_context_service_, &pref_service_),
            AtMemoryInvocationCustomizationSettingVisibility::kInvisible);
}

// Tests setting visibility when the toggle pref is off.
TEST_F(AtMemoryEnablementUtilsTest,
       InvocationCustomizationSettingVisibility_ToggleOff) {
  EXPECT_CALL(personal_context_service_, GetEnablementState)
      .WillRepeatedly(
          Return(personal_context::PersonalContextEnablementState::kEnabled));
  pref_service_.SetUserPref(
      personal_context::prefs::kPersonalContextInAutofillSettingsToggleStatus,
      base::Value(false));

  EXPECT_EQ(
      GetAtMemoryInvocationCustomizationSettingVisibility(
          &personal_context_service_, &pref_service_),
      AtMemoryInvocationCustomizationSettingVisibility::kVisibleGreyedOut);
}

// Tests that IsAtMemoryEnabled is false when personal_context_service is not
// eligible.
TEST_F(AtMemoryEnablementUtilsTest, IsAtMemoryEnabled_NotSupported) {
  EXPECT_CALL(personal_context_service_, GetEnablementState)
      .WillRepeatedly(Return(personal_context::PersonalContextEnablementState::
                                 kDisabledNotEligible));
  EXPECT_FALSE(IsAtMemoryEnabled(&personal_context_service_, &pref_service_));
}

// Tests that IsAtMemoryEnabled is false when personal_context_service is null.
TEST_F(AtMemoryEnablementUtilsTest,
       IsAtMemoryEnabled_NullPersonalContextService) {
  EXPECT_FALSE(IsAtMemoryEnabled(nullptr, &pref_service_));
}

// Tests that IsAtMemoryEnabled is false when pref_service is null.
TEST_F(AtMemoryEnablementUtilsTest, IsAtMemoryEnabled_NullPrefService) {
  EXPECT_CALL(personal_context_service_, GetEnablementState)
      .WillRepeatedly(
          Return(personal_context::PersonalContextEnablementState::kEnabled));
  EXPECT_FALSE(IsAtMemoryEnabled(&personal_context_service_, nullptr));
}

// Tests that IsAtMemoryEnabled is false when the client supports At-Memory but
// the settings toggle is disabled.
TEST_F(AtMemoryEnablementUtilsTest, IsAtMemoryEnabled_SupportedButToggleOff) {
  EXPECT_CALL(personal_context_service_, GetEnablementState)
      .WillRepeatedly(
          Return(personal_context::PersonalContextEnablementState::kEnabled));
  pref_service_.SetUserPref(
      personal_context::prefs::kPersonalContextInAutofillSettingsToggleStatus,
      base::Value(false));
  EXPECT_FALSE(IsAtMemoryEnabled(&personal_context_service_, &pref_service_));
}

// Tests that IsAtMemoryEnabled is true when the client supports At-Memory and
// the settings toggle is enabled.
TEST_F(AtMemoryEnablementUtilsTest, IsAtMemoryEnabled_SupportedAndToggleOn) {
  EXPECT_CALL(personal_context_service_, GetEnablementState)
      .WillRepeatedly(
          Return(personal_context::PersonalContextEnablementState::kEnabled));
  pref_service_.SetUserPref(
      personal_context::prefs::kPersonalContextInAutofillSettingsToggleStatus,
      base::Value(true));
  EXPECT_TRUE(IsAtMemoryEnabled(&personal_context_service_, &pref_service_));
}

}  // namespace
}  // namespace autofill
