// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/dictation/dictation_keyed_service.h"

#include "base/feature_list.h"
#include "base/functional/bind.h"
#include "base/strings/utf_string_conversions.h"
#include "chrome/browser/dictation/dictation_keyed_service_factory.h"
#include "chrome/browser/dictation/features.h"
#include "chrome/browser/dictation/listener_stream_provider.h"
#include "chrome/browser/dictation/session_controller.h"
#include "chrome/browser/dictation/session_ui_impl.h"
#include "chrome/browser/dictation/target.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser_window/public/browser_window_interface.h"
#include "chrome/common/pref_names.h"
#include "components/prefs/pref_service.h"

namespace dictation {

namespace {
constexpr int kVoiceTypingSettingsDisabled = 2;
}  // namespace

// static
DictationKeyedService* DictationKeyedService::Get(
    content::BrowserContext* context) {
  return DictationKeyedServiceFactory::GetDictationKeyedService(context);
}

DictationKeyedService::SessionState::SessionState(
    SessionControllerDelegate& delegate,
    base::WeakPtr<BrowserWindowInterface> window)
    : controller_(delegate), window_(window) {}

DictationKeyedService::SessionState::~SessionState() = default;

DictationKeyedService::DictationKeyedService(Profile* profile)
    : profile_(profile) {
  CHECK(base::FeatureList::IsEnabled(kDictation));
  pref_change_registrar_.Init(profile_->GetPrefs());
  pref_change_registrar_.Add(
      prefs::kVoiceTypingSettings,
      base::BindRepeating(&DictationKeyedService::OnPrefChanged,
                          base::Unretained(this)));
}

DictationKeyedService::~DictationKeyedService() = default;

void DictationKeyedService::Shutdown() {
  EndSession();
}

std::unique_ptr<StreamProvider> DictationKeyedService::CreateStreamProvider(
    SessionController& controller) const {
  return std::make_unique<ListenerStreamProvider>(profile_, controller);
}

std::unique_ptr<SessionUi> DictationKeyedService::CreateUi(
    SessionController& controller) const {
  CHECK(session_);
  if (!session_->window_) {
    return nullptr;
  }

  return std::make_unique<SessionUiImpl>(*session_->window_, controller);
}

void DictationKeyedService::StartSession(BrowserWindowInterface& window,
                                         std::unique_ptr<Target> target) {
  CHECK(!IsDisabledByPolicy());
  CHECK(!session_);

  session_.emplace(*this, window.GetWeakPtr());

  session_->controller_.Initialize();

  if (target) {
    session_->controller_.StartDictationStream(std::move(target));
  }
}

void DictationKeyedService::EndSession() {
  session_.reset();
}

bool DictationKeyedService::ShouldShowContextMenuItem() const {
  if (IsDisabledByPolicy()) {
    return false;
  }
  return !session_;
}

void DictationKeyedService::ContextMenuHandler(
    BrowserWindowInterface& window,
    const std::u16string& selected_text) {
  // Policy could have changed to disabled while the context menu was open.
  if (IsDisabledByPolicy()) {
    return;
  }

  // TODO(crbug.com/508729855) Populate target with information about the
  // targeted field from context menu params.
  StartSession(window,
               std::make_unique<Target>(base::UTF16ToUTF8(selected_text)));
}

bool DictationKeyedService::IsDisabledByPolicy() const {
  CHECK(profile_);
  return profile_->GetPrefs()->GetInteger(prefs::kVoiceTypingSettings) ==
         kVoiceTypingSettingsDisabled;
}

void DictationKeyedService::OnPrefChanged() {
  if (IsDisabledByPolicy()) {
    EndSession();
  }
}

}  // namespace dictation
