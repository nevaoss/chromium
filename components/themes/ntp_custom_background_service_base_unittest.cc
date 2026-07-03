// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/themes/ntp_custom_background_service_base.h"

#include <memory>

#include "components/application_locale_storage/application_locale_storage.h"
#include "components/themes/ntp_background_service.h"
#include "components/themes/ntp_custom_background_service_constants.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"
#include "base/files/file_path.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/prefs/testing_pref_service.h"
#include "components/themes/ntp_custom_background_service_observer.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {
const char kTestDictPref[] = "dict_pref";
const char kTestLocalPref[] = "local_pref";
const char kTestLocale[] = "en-US";
const char kTestBackgroundUrlKey[] = "background_url";
const char kTestBackgroundUrl[] = "https://example.com/image.jpg";
const char kValidBackgroundUrl[] = "https://www.foo.com/";
const char kValidActionUrl[] = "https://action.com/";
const char kAttributionLine1[] = "line1";
const char kAttributionLine2[] = "line2";
const char kCollectionId[] = "collection";
}  // namespace

class MockNtpCustomBackgroundServiceObserver
    : public NtpCustomBackgroundServiceObserver {
 public:
  MOCK_METHOD(void, OnCustomBackgroundImageUpdated, (), (override));
  MOCK_METHOD(void, OnNtpCustomBackgroundServiceShuttingDown, (), (override));
};

class TestNtpCustomBackgroundServiceBase
    : public NtpCustomBackgroundServiceBase {
 public:
  TestNtpCustomBackgroundServiceBase(PrefService* pref_service,
                                     NtpBackgroundService* background_service)
      : NtpCustomBackgroundServiceBase(pref_service,
                                       background_service,
                                       kTestDictPref,
                                       kTestLocalPref) {}

  using NtpCustomBackgroundServiceBase::IsCustomBackgroundPrefValid;
  using NtpCustomBackgroundServiceBase::NotifyAboutBackgrounds;

  void SelectLocalBackgroundImage(const base::FilePath& path) override {}
};

class NtpCustomBackgroundServiceBaseTest : public testing::Test {
 public:
  NtpCustomBackgroundServiceBaseTest() = default;

  void SetUp() override {
    pref_service_ = std::make_unique<TestingPrefServiceSimple>();
    pref_service_->registry()->RegisterDictionaryPref(kTestDictPref);
    pref_service_->registry()->RegisterBooleanPref(kTestLocalPref, false);

    application_locale_storage_ = std::make_unique<ApplicationLocaleStorage>();
    application_locale_storage_->Set(kTestLocale);
    background_service_ = std::make_unique<NtpBackgroundService>(
        application_locale_storage_.get(), nullptr);

    service_ = std::make_unique<TestNtpCustomBackgroundServiceBase>(
        pref_service_.get(), background_service_.get());
  }

  void TearDown() override {
    service_.reset();
    background_service_.reset();
    application_locale_storage_.reset();
    pref_service_.reset();
  }

 protected:
  std::unique_ptr<TestingPrefServiceSimple> pref_service_;
  std::unique_ptr<ApplicationLocaleStorage> application_locale_storage_;
  std::unique_ptr<NtpBackgroundService> background_service_;
  std::unique_ptr<TestNtpCustomBackgroundServiceBase> service_;
};

TEST_F(NtpCustomBackgroundServiceBaseTest, Initialization) {
  EXPECT_NE(service_, nullptr);
}

TEST_F(NtpCustomBackgroundServiceBaseTest, Observers) {
  testing::StrictMock<MockNtpCustomBackgroundServiceObserver> observer;
  service_->AddObserver(&observer);

  EXPECT_CALL(observer, OnCustomBackgroundImageUpdated());
  service_->NotifyAboutBackgrounds();

  service_->RemoveObserver(&observer);
  // StrictMock will fail if the observer is still notified.
  service_->NotifyAboutBackgrounds();
}

TEST_F(NtpCustomBackgroundServiceBaseTest, ResetCustomBackgroundInfo) {
  pref_service_->SetBoolean(kTestLocalPref, true);
  base::DictValue dict;
  dict.Set(kTestBackgroundUrlKey, base::Value(kTestBackgroundUrl));
  pref_service_->SetDict(kTestDictPref, std::move(dict));

  service_->ResetCustomBackgroundInfo();

  EXPECT_FALSE(pref_service_->GetBoolean(kTestLocalPref));
  EXPECT_TRUE(pref_service_->GetDict(kTestDictPref).empty());
}

TEST_F(NtpCustomBackgroundServiceBaseTest, IsCustomBackgroundPrefValid) {
  EXPECT_FALSE(service_->IsCustomBackgroundPrefValid());

  base::DictValue dict;
  dict.Set(kTestBackgroundUrlKey, base::Value(kTestBackgroundUrl));
  pref_service_->SetDict(kTestDictPref, std::move(dict));

  EXPECT_TRUE(service_->IsCustomBackgroundPrefValid());
}

TEST_F(NtpCustomBackgroundServiceBaseTest, SetCustomBackgroundInfo) {
  const GURL kUrl(kValidBackgroundUrl);
  background_service_->AddValidBackdropUrlForTesting(kUrl);

  service_->SetCustomBackgroundInfo(kUrl, GURL(), kAttributionLine1, kAttributionLine2,
                                    GURL(kValidActionUrl), kCollectionId);

  const base::DictValue& dict = pref_service_->GetDict(kTestDictPref);
  EXPECT_FALSE(dict.empty());

  const std::string* url = dict.FindString(kNtpCustomBackgroundURL);
  ASSERT_TRUE(url);
  EXPECT_EQ(*url, kValidBackgroundUrl);

  const std::string* collection_id = dict.FindString(kNtpCustomBackgroundCollectionId);
  ASSERT_TRUE(collection_id);
  EXPECT_EQ(*collection_id, kCollectionId);
}
