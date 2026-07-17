// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/dictation/test_util.h"

#include "base/functional/bind.h"
#include "base/path_service.h"
#include "base/run_loop.h"
#include "base/test/bind.h"
#include "chrome/browser/dictation/dictation_multiplexer.h"
#include "chrome/browser/dictation/listener_stream_provider.h"
#include "chrome/browser/extensions/chrome_test_extension_loader.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/common/chrome_paths.h"
#include "content/public/test/browser_test_utils.h"
#include "extensions/browser/browsertest_util.h"
#include "extensions/browser/extension_registry_test_helper.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace dictation {

const extensions::Extension* LoadTestExtensionInManualMode(Profile* profile) {
  extensions::ExtensionRegistryTestHelper observer(
      std::string(kDictationTestExtensionId).c_str(), profile);
  base::FilePath test_data_dir;
  EXPECT_TRUE(base::PathService::Get(chrome::DIR_TEST_DATA, &test_data_dir));
  base::FilePath extension_path =
      test_data_dir.AppendASCII("extensions").AppendASCII("dictation");
  extensions::ChromeTestExtensionLoader loader(profile);
  const extensions::Extension* ext = loader.LoadExtension(extension_path).get();
  EXPECT_TRUE(ext);
  EXPECT_EQ(ext->id(), kDictationTestExtensionId);
  observer.WaitForServiceWorkerStart();

  std::string script = R"JS(
    (async function() {
      await chrome.storage.local.set({manualTest: true});
      chrome.test.sendScriptResult('ready');
    })();
  )JS";

  base::Value result =
      extensions::browsertest_util::ExecuteScriptInBackgroundPage(
          profile, std::string(kDictationTestExtensionId), script);
  EXPECT_EQ("ready", result);

  return ext;
}

void ExtensionSendTranscriptUpdate(
    Profile* profile,
    DictationMultiplexer::StreamId stream_id,
    extensions::api::dictation_private::TranscriptionType type,
    std::string_view data) {
  std::string script = content::JsReplace(
      R"JS(
    (async function() {
      try {
        await chrome.dictationPrivate.updateTranscription({
          streamId: $1,
          type: $2,
          data: $3
        });
        chrome.test.sendScriptResult('success');
      } catch (e) {
        chrome.test.sendScriptResult('error: ' + e.message);
      }
    })();
  )JS",
      stream_id.value(), extensions::api::dictation_private::ToString(type),
      data);

  base::Value result =
      extensions::browsertest_util::ExecuteScriptInBackgroundPage(
          profile, std::string(kDictationTestExtensionId), script);
  CHECK_EQ("success", result.GetString());
}

void ExtensionSendStreamStateUpdate(
    Profile* profile,
    DictationMultiplexer::StreamId stream_id,
    extensions::api::dictation_private::StreamState state) {
  std::string script = content::JsReplace(
      R"JS(
    (async function() {
      try {
        await chrome.dictationPrivate.setStreamState({
            streamId: $1,
            state: $2
        });
        chrome.test.sendScriptResult('success');
      } catch (e) {
        chrome.test.sendScriptResult('error: ' + e.message);
      }
    })();
      )JS",
      stream_id.value(), extensions::api::dictation_private::ToString(state));

  base::Value result =
      extensions::browsertest_util::ExecuteScriptInBackgroundPage(
          profile, std::string(kDictationTestExtensionId), script);
  CHECK_EQ("success", result.GetString());
}

using ::testing::_;

MockStreamProvider::MockStreamProvider() = default;
MockStreamProvider::~MockStreamProvider() = default;

MockSessionUi::MockSessionUi() = default;
MockSessionUi::~MockSessionUi() = default;

MockSessionControllerDelegate::MockSessionControllerDelegate() {
  ON_CALL(*this, CreateUi(_)).WillByDefault([]() {
    return std::make_unique<testing::NiceMock<MockSessionUi>>();
  });
  ON_CALL(*this, CreateStreamProvider(_)).WillByDefault([]() {
    return std::make_unique<testing::NiceMock<MockStreamProvider>>();
  });
}
MockSessionControllerDelegate::~MockSessionControllerDelegate() = default;

MockTarget::MockTarget() = default;
MockTarget::~MockTarget() = default;

}  // namespace dictation
