// Copyright 2017 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include "ui/base/ime/linux/neva/input_method_auralinux_neva.h"

#include "ui/base/ime/text_input_client.h"
#include "ui/base/ime/text_input_type.h"
#include "ui/events/keycodes/dom/dom_code.h"
#include "ui/events/keycodes/dom/dom_key.h"

namespace ui {

InputMethodAuraLinuxNeva::InputMethodAuraLinuxNeva(
    ImeKeyEventDispatcher* ime_key_event_dispatcher,
    gfx::AcceleratedWidget widget)
    : InputMethodAuraLinux(ime_key_event_dispatcher, widget) {}

bool InputMethodAuraLinuxNeva::IsPasswordInputFieldFocused() {
  return GetTextInputType() == ui::TEXT_INPUT_TYPE_PASSWORD;
}

EventDispatchDetails InputMethodAuraLinuxNeva::SendFakeKeyEvent(
    uint32_t key_value,
    ui::EventType type) const {
  ui::KeyEvent char_event(
      type, ui::VKEY_UNKNOWN, ui::DomCode::NONE, ui::EF_IME_FABRICATED_KEY,
      ui::DomKey::FromCharacter(key_value), base::TimeTicks(), false);

  EventDispatchDetails details = DispatchKeyEventPostIME(&char_event);

  if (char_event.stopped_propagation()) {
    char_event.StopPropagation();
  }

  return details;
}

void InputMethodAuraLinuxNeva::CancelComposition(
    const TextInputClient* client) {
  if (!IsTextInputClientFocused(client)) {
    return;
  }

  // In some password input fields, such as pin pad, a button press event is
  // expected rather than text.
  if (IsPasswordInputFieldFocused() && !composition_.text.empty()) {
    SendFakeKeyEvent(static_cast<uint32_t>(composition_.text[0]),
                     ui::EventType::kKeyPressed);
  }

  ResetContext();
}

void InputMethodAuraLinuxNeva::OnDeleteRange(int32_t index, uint32_t length) {
  if (IsTextInputTypeNone())
    return;

  TextInputClient* client = GetTextInputClient();
  if (client) {
    gfx::Range range;
    bool res = false;
    if (length != std::numeric_limits<uint32_t>::max()) {
      res = client->GetEditableSelectionRange(&range);
      if (res) {
        // In the case of composition, we should exclude composition range from
        // the selection range. This is required after upgrade to v79 chromium.
        // Ime manager handles prediction without composition.
        gfx::Range composition_range;
        gfx::Range surround_range = range;
        if (client->GetCompositionTextRange(&composition_range) &&
            range.IsBoundedBy(composition_range)) {
          surround_range.set_start(
              std::min(range.GetMin(), composition_range.GetMin()));
          surround_range.set_end(
              std::min(range.GetMax(), composition_range.GetMin()));
        }
        range.set_start(surround_range.start() + index);
        range.set_end(surround_range.start() + index + length);
      }
    } else {
      res = client->GetTextRange(&range);
    }

    if (res && range.IsValid())
      client->DeleteRange(range);
  }
}

void InputMethodAuraLinuxNeva::OnCommit(const std::u16string& text) {
  // In case Neva Appruntime we should reset suppress_non_key_input_until_ for
  // OnCommit handling. Neva Appruntime VKB calls OnCommit without key event
  // processing in InputMethod and therefore the first call of OnCommit is
  // ignored.
  suppress_non_key_input_until_ = base::TimeTicks();
  InputMethodAuraLinux::OnCommit(text);

  if (IgnoringNonKeyInput() || IsTextInputTypeNone() || is_sync_mode_ ||
      !GetTextInputClient())
    return;

  // In some password input fields, such as pin pad, a button press event is
  // expected rather than text.
  if ((text.length() == 1) && IsPasswordInputFieldFocused()) {
    uint32_t key_value = static_cast<uint32_t>(text[0]);
    // Send both key down and key up
    EventDispatchDetails details =
        SendFakeKeyEvent(key_value, ui::EventType::kKeyPressed);
    if (details.dispatcher_destroyed) {
      return;
    }

    details = SendFakeKeyEvent(key_value, ui::EventType::kKeyReleased);
    if (details.dispatcher_destroyed) {
      return;
    }
  } else {
    EventDispatchDetails details = SendFakeReleaseKeyEvent();
    if (details.dispatcher_destroyed) {
      return;
    }
  }
}

void InputMethodAuraLinuxNeva::OnPreeditChanged(const CompositionText&
    composition_text) {
  if (IgnoringNonKeyInput() || IsTextInputTypeNone() || is_sync_mode_ ||
      (composition_.text.empty() && composition_text.text.empty()))
    return;
  InputMethodAuraLinux::OnPreeditChanged(composition_text);
  EventDispatchDetails details = SendFakeReleaseKeyEvent();
  if (details.dispatcher_destroyed)
    return;
}

void InputMethodAuraLinuxNeva::OnPreeditEnd() {
  InputMethodAuraLinux::OnPreeditEnd();
  TextInputClient* client = GetTextInputClient();
  if (IgnoringNonKeyInput() || IsTextInputTypeNone() || is_sync_mode_ ||
      !client || !client->HasCompositionText())
    return;
  EventDispatchDetails details = SendFakeReleaseKeyEvent();
  if (details.dispatcher_destroyed)
    return;
}

EventDispatchDetails InputMethodAuraLinuxNeva::SendFakeReleaseKeyEvent() const {
  KeyEvent key_event(EventType::kKeyReleased, VKEY_PROCESSKEY, 0);
  EventDispatchDetails details = DispatchKeyEventPostIME(&key_event);
  if (key_event.stopped_propagation())
    key_event.StopPropagation();
  return details;
}

}  // namespace ui
