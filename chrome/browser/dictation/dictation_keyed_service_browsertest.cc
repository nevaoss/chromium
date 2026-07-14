// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/dictation/dictation_keyed_service.h"

#include "base/path_service.h"
#include "base/test/bind.h"
#include "base/test/scoped_feature_list.h"
#include "chrome/app/chrome_command_ids.h"
#include "chrome/browser/dictation/dictation_keyed_service_factory.h"
#include "chrome/browser/dictation/features.h"
#include "chrome/browser/dictation/listener_stream_provider.h"
#include "chrome/browser/dictation/target.h"
#include "chrome/browser/extensions/chrome_test_extension_loader.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/renderer_context_menu/render_view_context_menu_test_util.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/common/chrome_paths.h"
#include "chrome/test/base/chrome_test_utils.h"
#include "chrome/test/base/platform_browser_test.h"
#include "components/tabs/public/tab_interface.h"
#include "content/public/test/browser_test.h"
#include "content/public/test/browser_test_utils.h"
#include "extensions/browser/browsertest_util.h"
#include "extensions/browser/extension_registry_test_helper.h"
#include "extensions/common/switches.h"

namespace dictation {

namespace {

constexpr std::string_view kDictationTestExtensionId =
    "dfihfgggpgemecjdjahibncmmjlfjggp";

class DictationKeyedServiceBrowserTest : public PlatformBrowserTest {
 public:
  DictationKeyedServiceBrowserTest() {
    scoped_feature_list_.InitAndEnableFeature(kDictation);
  }
  ~DictationKeyedServiceBrowserTest() override = default;

  void SetUpCommandLine(base::CommandLine* command_line) override {
    PlatformBrowserTest::SetUpCommandLine(command_line);
    command_line->AppendSwitchASCII(
        extensions::switches::kAllowlistedExtensionID,
        kDictationTestExtensionId);
  }

  Profile* profile() { return chrome_test_utils::GetProfile(this); }

  content::WebContents* web_contents() {
    return chrome_test_utils::GetActiveWebContents(this);
  }

  const extensions::Extension* LoadTestExtension() {
    extensions::ExtensionRegistryTestHelper observer(
        std::string(kDictationTestExtensionId).c_str(), profile());
    base::FilePath test_data_dir;
    EXPECT_TRUE(base::PathService::Get(chrome::DIR_TEST_DATA, &test_data_dir));
    base::FilePath extension_path =
        test_data_dir.AppendASCII("extensions").AppendASCII("dictation");
    extensions::ChromeTestExtensionLoader loader(profile());
    const extensions::Extension* ext =
        loader.LoadExtension(extension_path).get();
    EXPECT_TRUE(ext);
    EXPECT_EQ(ext->id(), kDictationTestExtensionId);
    observer.WaitForServiceWorkerStart();
    return ext;
  }

  void SetTranscript(const std::string& transcript) {
    std::string script = content::JsReplace(R"JS(
      (async function() {
        await chrome.storage.local.set({cannedResponse: $1});
        chrome.test.sendScriptResult('ready');
      })();
    )JS",
                                            transcript);

    base::Value result =
        extensions::browsertest_util::ExecuteScriptInBackgroundPage(
            profile(), std::string(kDictationTestExtensionId), script);
    EXPECT_EQ("ready", result);
  }

 private:
  base::test::ScopedFeatureList scoped_feature_list_;
};

IN_PROC_BROWSER_TEST_F(DictationKeyedServiceBrowserTest,
                       CreatedForRegularProfile) {
  EXPECT_NE(DictationKeyedService::Get(profile()), nullptr);
}

IN_PROC_BROWSER_TEST_F(DictationKeyedServiceBrowserTest,
                       NotCreatedForOTRProfile) {
  Profile* otr_profile =
      profile()->GetPrimaryOTRProfile(/*create_if_needed=*/true);
  EXPECT_EQ(DictationKeyedService::Get(otr_profile), nullptr);
}

class DictationKeyedServiceDisabledBrowserTest
    : public DictationKeyedServiceBrowserTest {
 public:
  DictationKeyedServiceDisabledBrowserTest() {
    scoped_feature_list_.InitAndDisableFeature(kDictation);
  }
  ~DictationKeyedServiceDisabledBrowserTest() override = default;

 private:
  base::test::ScopedFeatureList scoped_feature_list_;
};

IN_PROC_BROWSER_TEST_F(DictationKeyedServiceDisabledBrowserTest,
                       NotCreatedWhenDisabled) {
  EXPECT_EQ(DictationKeyedService::Get(profile()), nullptr);
}

IN_PROC_BROWSER_TEST_F(DictationKeyedServiceBrowserTest,
                       ShouldShowContextMenuItem) {
  DictationKeyedService* service = DictationKeyedService::Get(profile());
  ASSERT_NE(service, nullptr);

  EXPECT_TRUE(service->ShouldShowContextMenuItem());

  service->StartSession(*GetBrowserWindowInterface(), nullptr);

  EXPECT_FALSE(service->ShouldShowContextMenuItem());

  service->EndSession();

  EXPECT_TRUE(service->ShouldShowContextMenuItem());
}

IN_PROC_BROWSER_TEST_F(DictationKeyedServiceBrowserTest,
                       ExecuteContextMenuCommand) {
  DictationKeyedService* service = DictationKeyedService::Get(profile());
  ASSERT_NE(service, nullptr);

  content::ContextMenuParams params;
  params.is_editable = true;
  TestRenderViewContextMenu menu(*web_contents()->GetPrimaryMainFrame(),
                                 params);
  menu.Init();

  ASSERT_TRUE(menu.IsItemPresent(IDC_CONTENT_CONTEXT_DICTATION));
  ASSERT_TRUE(menu.IsItemEnabled(IDC_CONTENT_CONTEXT_DICTATION));

  menu.ExecuteCommand(IDC_CONTENT_CONTEXT_DICTATION, 0);

  EXPECT_NE(service->session_controller(), nullptr);
}

// TODO(crbug.com/502587072): Add tests which have the test extension simulate
// stream failures, including on start and mid stream.

IN_PROC_BROWSER_TEST_F(DictationKeyedServiceBrowserTest,
                       StartSessionAndReceiveTranscription) {
  LoadTestExtension();
  SetTranscript("Hello world");

  DictationKeyedService* service = DictationKeyedService::Get(profile());
  ASSERT_NE(service, nullptr);

  service->StartSession(*GetBrowserWindowInterface(),
                        std::make_unique<Target>());

  ListenerStreamProvider* provider = static_cast<ListenerStreamProvider*>(
      service->session_controller()->attached_stream_provider());
  base::RunLoop wait_for_updates_loop;
  base::RunLoop stream_complete_loop;
  bool seen_partial = false;
  provider->SetOnUpdateForTesting(base::BindLambdaForTesting([&]() {
    if (!seen_partial) {
      if (provider->GetLatestTranscriptionForTesting() == "Hello") {
        EXPECT_FALSE(provider->IsTranscriptionFinalForTesting());
        EXPECT_EQ(provider->GetStateForTesting(),
                  StreamProvider::StreamState::kTranscribing);
        seen_partial = true;
      }
    }
    if (provider->GetLatestTranscriptionForTesting() == "Hello world" &&
        provider->IsTranscriptionFinalForTesting()) {
      EXPECT_TRUE(seen_partial);
      wait_for_updates_loop.Quit();
    }
    if (provider->GetStateForTesting() ==
        StreamProvider::StreamState::kComplete) {
      stream_complete_loop.Quit();
    }
  }));
  wait_for_updates_loop.Run();

  provider->Stop();

  stream_complete_loop.Run();

  service->EndSession();
}

}  // namespace

}  // namespace dictation
