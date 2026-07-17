// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/autofill/core/browser/integrators/personal_context/personal_context_autofill_util.h"

#include "base/test/scoped_feature_list.h"
#include "build/branding_buildflags.h"
#include "components/autofill/core/common/autofill_features.h"
#include "components/personal_context/core/personal_context_enablement_service.h"
#include "components/personal_context/core/personal_context_types.h"
#include "components/prefs/testing_pref_service.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace autofill {

namespace {

using personal_context::PersonalContextEnablementService;
using personal_context::PersonalContextEnablementState;
using ::testing::NiceMock;
using ::testing::Return;

class MockPersonalContextEnablementService
    : public PersonalContextEnablementService {
 public:
  MOCK_METHOD(void, AddObserver, (Observer*), (override));
  MOCK_METHOD(void, RemoveObserver, (Observer*), (override));
  MOCK_METHOD(PersonalContextEnablementState,
              GetEnablementState,
              (),
              (override));
};

}  // namespace

TEST(PersonalContextAutofillUtilTest,
     ShouldShowPersonalContextAutofillSetting) {
  using enum PersonalContextEnablementState;
  NiceMock<MockPersonalContextEnablementService> service;

  base::test::ScopedFeatureList feature_list(
      features::kAutofillAmbientAutofill);

  auto check_state = [&](PersonalContextEnablementState state) {
    ON_CALL(service, GetEnablementState()).WillByDefault(Return(state));
    return ShouldShowPersonalContextAutofillSetting(
        &service, /*subscription_eligibility_service=*/nullptr,
        /*pref_service=*/nullptr, /*google_groups_manager=*/nullptr);
  };

  EXPECT_FALSE(check_state(kDisabledNotEligible));
  EXPECT_FALSE(check_state(kDisabledNeedsOptIn));
  EXPECT_TRUE(check_state(kDisabledViaPersonalIntelligenceInAutofillToggle));
  EXPECT_TRUE(check_state(kEnabledShouldShowNotice));
  EXPECT_TRUE(check_state(kEnabled));

  EXPECT_FALSE(ShouldShowPersonalContextAutofillSetting(
      /*enablement_service=*/nullptr,
      /*subscription_eligibility_service=*/nullptr,
      /*pref_service=*/nullptr, /*google_groups_manager=*/nullptr));
}

TEST(PersonalContextAutofillUtilTest,
     ShouldShowPersonalContextAutofillSetting_BothDisabled) {
  NiceMock<MockPersonalContextEnablementService> service;
  ON_CALL(service, GetEnablementState())
      .WillByDefault(Return(PersonalContextEnablementState::kEnabled));

  base::test::ScopedFeatureList feature_list;
  feature_list.InitWithFeatures(
      /*enabled_features=*/{},
      /*disabled_features=*/{features::kAutofillAmbientAutofill,
                             features::kAutofillAtMemory});
  EXPECT_FALSE(ShouldShowPersonalContextAutofillSetting(
      &service, /*subscription_eligibility_service=*/nullptr,
      /*pref_service=*/nullptr, /*google_groups_manager=*/nullptr));
}

TEST(PersonalContextAutofillUtilTest,
     ShouldShowPersonalContextAutofillSetting_AmbientAutofillEnabled) {
  NiceMock<MockPersonalContextEnablementService> service;
  ON_CALL(service, GetEnablementState())
      .WillByDefault(Return(PersonalContextEnablementState::kEnabled));

  base::test::ScopedFeatureList feature_list;
  feature_list.InitWithFeatures(
      /*enabled_features=*/{features::kAutofillAmbientAutofill},
      /*disabled_features=*/{features::kAutofillAtMemory});
  EXPECT_TRUE(ShouldShowPersonalContextAutofillSetting(
      &service, /*subscription_eligibility_service=*/nullptr,
      /*pref_service=*/nullptr, /*google_groups_manager=*/nullptr));
}

#if BUILDFLAG(GOOGLE_CHROME_BRANDING)
TEST(PersonalContextAutofillUtilTest,
     ShouldShowPersonalContextAutofillSetting_AtMemoryEnabled) {
  NiceMock<MockPersonalContextEnablementService> service;
  ON_CALL(service, GetEnablementState())
      .WillByDefault(Return(PersonalContextEnablementState::kEnabled));

  base::test::ScopedFeatureList feature_list;
  feature_list.InitWithFeatures(
      /*enabled_features=*/{features::kAutofillAtMemory},
      /*disabled_features=*/{features::kAutofillAmbientAutofill});
  EXPECT_TRUE(ShouldShowPersonalContextAutofillSetting(
      &service, /*subscription_eligibility_service=*/nullptr,
      /*pref_service=*/nullptr, /*google_groups_manager=*/nullptr));
}
#endif  // BUILDFLAG(GOOGLE_CHROME_BRANDING)

}  // namespace autofill
