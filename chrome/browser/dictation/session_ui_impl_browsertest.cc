// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/dictation/session_ui_impl.h"

#include "base/memory/weak_ptr.h"
#include "base/test/scoped_feature_list.h"
#include "chrome/browser/dictation/dictation_keyed_service.h"
#include "chrome/browser/dictation/features.h"
#include "chrome/browser/dictation/listener_stream_provider.h"
#include "chrome/browser/dictation/session_state.h"
#include "chrome/browser/dictation/session_ui.h"
#include "chrome/browser/dictation/target.h"
#include "chrome/browser/dictation/test_util.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_element_identifiers.h"
#include "chrome/browser/ui/views/dictation/dictation_bubble_ui.h"
#include "chrome/common/extensions/api/dictation_private.h"
#include "chrome/test/base/chrome_test_utils.h"
#include "chrome/test/interaction/interactive_browser_test.h"
#include "content/public/test/browser_test.h"
#include "extensions/common/switches.h"
#include "ui/base/interaction/element_tracker.h"
#include "ui/views/controls/button/label_button.h"

namespace dictation {

using ExtensionStreamState = extensions::api::dictation_private::StreamState;
using ExtensionTranscriptionType =
    extensions::api::dictation_private::TranscriptionType;

class DictationSessionUiImplBrowserTest : public InteractiveBrowserTest {
 public:
  DictationSessionUiImplBrowserTest() {
    scoped_feature_list_.InitAndEnableFeature(kDictation);
  }
  ~DictationSessionUiImplBrowserTest() override = default;

  void SetUpCommandLine(base::CommandLine* command_line) override {
    InteractiveBrowserTest::SetUpCommandLine(command_line);
    command_line->AppendSwitchASCII(
        extensions::switches::kAllowlistedExtensionID,
        kDictationTestExtensionId);
  }

  void SetUpOnMainThread() override {
    InteractiveBrowserTest::SetUpOnMainThread();
    LoadTestExtensionInManualMode(profile());
  }

  void TearDownOnMainThread() override {
    dictation_service().EndSession();
    InteractiveBrowserTest::TearDownOnMainThread();
  }

  Profile* profile() { return chrome_test_utils::GetProfile(this); }

  DictationKeyedService& dictation_service() {
    return *DictationKeyedService::Get(profile());
  }

  SessionUiImpl* session_ui() {
    if (!dictation_service().session_controller()) {
      return nullptr;
    }

    return static_cast<SessionUiImpl*>(
        dictation_service().session_controller()->ui_for_testing());
  }

  auto StartSession() {
    // clang-format off
    return Steps(
      Do([this]{
        dictation_service().StartSession(
            *browser(),
            std::make_unique<Target>());
        last_started_provider_ = static_cast<ListenerStreamProvider*>(
            dictation_service()
                .session_controller()
                ->attached_stream_provider())->GetWeakPtr();
        ASSERT_NE(last_started_provider_, nullptr);
      }),
      Check([this]{ return session_ui() != nullptr; })
    );
    // clang-format on
  }

  auto ExtensionAPISetStreamState(ExtensionStreamState state) {
    return Steps(Do([this, state] {
      ASSERT_NE(last_started_provider_, nullptr);
      ExtensionSendStreamStateUpdate(
          profile(), last_started_provider_->stream_id_for_testing(), state);
    }));
  }

  auto ExtensionAPIUpdateTranscription(ExtensionTranscriptionType type,
                                       std::string_view text) {
    return Steps(Do([this, type, text_str = std::string(text)] {
      ASSERT_NE(last_started_provider_, nullptr);
      ExtensionSendTranscriptUpdate(
          profile(), last_started_provider_->stream_id_for_testing(), type,
          text_str);
    }));
  }

  auto GetSessionState() {
    return [this]() {
      return dictation_service().session_controller()->GetState();
    };
  }

  auto HasAttachedStreamProvider() {
    return [this]() {
      return dictation_service()
                 .session_controller()
                 ->attached_stream_provider() != nullptr;
    };
  }

 private:
  base::WeakPtr<ListenerStreamProvider> last_started_provider_ = nullptr;
  base::test::ScopedFeatureList scoped_feature_list_;
};

IN_PROC_BROWSER_TEST_F(DictationSessionUiImplBrowserTest,
                       SessionStateUpdatesToggleButton) {
  // clang-format off
  RunTestSequence(
    StartSession(),
    WaitForShow(DictationBubbleUi::kViewElementIdForTesting),

    // kStreamInitializing.
    CheckResult(GetSessionState(), SessionState::kStreamInitializing),
    CheckViewProperty(DictationBubbleUi::kToggleButtonElementIdForTesting,
                      &views::LabelButton::GetText, u"Done"),
    CheckViewProperty(DictationBubbleUi::kToggleButtonElementIdForTesting,
                      &views::View::GetEnabled, true),

    // kTranscribing.
    ExtensionAPISetStreamState(ExtensionStreamState::kTranscribing),
    CheckResult(GetSessionState(), SessionState::kTranscribing),
    CheckViewProperty(DictationBubbleUi::kToggleButtonElementIdForTesting,
                      &views::LabelButton::GetText, u"Done"),
    CheckViewProperty(DictationBubbleUi::kToggleButtonElementIdForTesting,
                      &views::View::GetEnabled, true),

    // kFinalizing.
    Do([this]{
      dictation_service().session_controller()->EndDictationStream();
    }),
    CheckResult(GetSessionState(), SessionState::kFinalizing),
    CheckViewProperty(DictationBubbleUi::kToggleButtonElementIdForTesting,
                      &views::LabelButton::GetText, u"Done"),
    CheckViewProperty(DictationBubbleUi::kToggleButtonElementIdForTesting,
                      &views::View::GetEnabled, false),

    // kInactive
    ExtensionAPISetStreamState(ExtensionStreamState::kComplete),
    CheckResult(GetSessionState(), SessionState::kInactive),
    CheckViewProperty(DictationBubbleUi::kToggleButtonElementIdForTesting,
                      &views::LabelButton::GetText, u"Start"),
    CheckViewProperty(DictationBubbleUi::kToggleButtonElementIdForTesting,
                      &views::View::GetEnabled, true)
  );
  // clang-format on
}

IN_PROC_BROWSER_TEST_F(DictationSessionUiImplBrowserTest,
                       EndSessionTearsDownUI) {
  // clang-format off
  RunTestSequence(
    StartSession(),
    PressButton(DictationBubbleUi::kCloseButtonElementIdForTesting),
    WaitForHide(DictationBubbleUi::kViewElementIdForTesting),
    Check([this]{ return session_ui() == nullptr; })
  );
  // clang-format on
}

IN_PROC_BROWSER_TEST_F(DictationSessionUiImplBrowserTest,
                       DoneButtonEndsActiveStream) {
  // clang-format off
  RunTestSequence(
    StartSession(),
    ExtensionAPISetStreamState(ExtensionStreamState::kTranscribing),
    CheckResult(GetSessionState(), SessionState::kTranscribing),

    PressButton(DictationBubbleUi::kToggleButtonElementIdForTesting),

    // TODO(b/525943882): Currently the controller immediately disposes of the
    // stream but it should really be going into a finalization state. Update
    // once this is fixed.
    CheckResult(GetSessionState(), testing::Ne(SessionState::kTranscribing)),
    CheckResult(HasAttachedStreamProvider(), false)
  );
  // clang-format on
}

}  // namespace dictation
