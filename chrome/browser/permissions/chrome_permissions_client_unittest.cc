// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/permissions/chrome_permissions_client.h"

#include <memory>

#include "chrome/test/base/chrome_render_view_host_test_harness.h"
#include "components/permissions/permission_request_manager.h"
#include "components/permissions/test/mock_permission_request.h"
#include "testing/gtest/include/gtest/gtest.h"

#if !BUILDFLAG(IS_ANDROID)
#include "base/test/scoped_feature_list.h"
#include "chrome/browser/ui/hats/hats_service_factory.h"  // nogncheck
#include "chrome/browser/ui/hats/mock_hats_service.h"     // nogncheck
#include "chrome/browser/ui/hats/survey_config.h"         // nogncheck
#include "components/permissions/constants.h"
#include "components/permissions/permission_hats_trigger_helper.h"
#include "components/prefs/pref_service.h"
#include "components/unified_consent/pref_names.h"
#endif

class ChromePermissionsClientTest : public ChromeRenderViewHostTestHarness {
 public:
  void SetUp() override {
    ChromeRenderViewHostTestHarness::SetUp();
#if !BUILDFLAG(IS_ANDROID)
    permissions::PermissionHatsTriggerHelper::SetIsTest();
#endif
    permissions::PermissionRequestManager::CreateForWebContents(web_contents());
  }
};

#if BUILDFLAG(IS_ANDROID)
TEST_F(ChromePermissionsClientTest,
       MaybeCreateMessageUINeverQuietForUpgradeToPrecise) {
  auto request = std::make_unique<permissions::MockPermissionRequest>(
      GURL(permissions::MockPermissionRequest::kDefaultOrigin),
      permissions::RequestType::kGeolocation,
      permissions::PermissionRequestGestureType::GESTURE,
      permissions::GeolocationPromptType::kUpgradeToPrecise);

  base::WeakPtr<permissions::PermissionPromptAndroid> dummy_prompt;

  auto* client = ChromePermissionsClient::GetInstance();
  auto message_ui =
      client->MaybeCreateMessageUI(web_contents(), *request, dummy_prompt);

  EXPECT_FALSE(message_ui);
}
#endif  // BUILDFLAG(IS_ANDROID)

#if !BUILDFLAG(IS_ANDROID)
TEST_F(ChromePermissionsClientTest, HaTSUrlReportedOnlyIfOptedIn) {
  base::test::ScopedFeatureList feature_list;
  feature_list.InitWithFeaturesAndParameters(
      {{permissions::features::kPermissionsPromptSurvey,
        {{"probability_vector", "1.0"},
         {"survey_display_time", "OnPromptResolved"},
         {"trigger_id", "pqEK9eaX30ugnJ3q1cK0UsVJTo1z"}}}},
      {});

  HatsServiceFactory::GetInstance()->SetTestingFactory(
      profile(), base::BindRepeating(&BuildMockHatsService));
  auto* mock_hats_service =
      static_cast<MockHatsService*>(HatsServiceFactory::GetForProfile(
          profile(), /*create_if_necessary=*/true));

  GURL kTestUrl("https://example.com");
  NavigateAndCommit(kTestUrl);

  // Case 1: User is opted out of URL-keyed anonymized data collection.
  profile()->GetPrefs()->SetBoolean(
      unified_consent::prefs::kUrlKeyedAnonymizedDataCollectionEnabled, false);

  EXPECT_CALL(*mock_hats_service,
              LaunchSurvey(kHatsSurveyTriggerPermissionsPrompt, testing::_,
                           testing::_, testing::_,
                           testing::Contains(testing::Pair(
                               permissions::kPermissionPromptSurveyUrlKey, "")),
                           testing::_, testing::_));

  ChromePermissionsClient::GetInstance()->TriggerPromptHatsSurveyIfEnabled(
      web_contents(), permissions::RequestType::kNotifications,
      permissions::PermissionAction::GRANTED,
      permissions::PermissionPromptDisposition::ANCHORED_BUBBLE,
      permissions::PermissionPromptDispositionReason::DEFAULT_FALLBACK,
      permissions::PermissionRequestGestureType::GESTURE, base::Minutes(1),
      /*is_post_prompt=*/true, kTestUrl, std::nullopt, CONTENT_SETTING_DEFAULT,
      base::DoNothing(), std::monostate());

  testing::Mock::VerifyAndClearExpectations(mock_hats_service);

  // Case 2: User is opted in to URL-keyed anonymized data collection.
  profile()->GetPrefs()->SetBoolean(
      unified_consent::prefs::kUrlKeyedAnonymizedDataCollectionEnabled, true);

  EXPECT_CALL(*mock_hats_service,
              LaunchSurvey(kHatsSurveyTriggerPermissionsPrompt, testing::_,
                           testing::_, testing::_,
                           testing::Contains(testing::Pair(
                               permissions::kPermissionPromptSurveyUrlKey,
                               kTestUrl.spec())),
                           testing::_, testing::_));

  ChromePermissionsClient::GetInstance()->TriggerPromptHatsSurveyIfEnabled(
      web_contents(), permissions::RequestType::kNotifications,
      permissions::PermissionAction::GRANTED,
      permissions::PermissionPromptDisposition::ANCHORED_BUBBLE,
      permissions::PermissionPromptDispositionReason::DEFAULT_FALLBACK,
      permissions::PermissionRequestGestureType::GESTURE, base::Minutes(1),
      /*is_post_prompt=*/true, kTestUrl, std::nullopt, CONTENT_SETTING_DEFAULT,
      base::DoNothing(), std::monostate());
}
#endif  // !BUILDFLAG(IS_ANDROID)
