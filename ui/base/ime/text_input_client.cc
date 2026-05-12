// Copyright 2011 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/base/ime/text_input_client.h"

#include <string_view>

///@name USE_NEVA_APPRUNTIME
///@{
#include "ui/gfx/geometry/rect.h"
///@}

namespace ui {

TextInputClient::~TextInputClient() {
}

///@name USE_NEVA_APPRUNTIME
///@{
bool TextInputClient::SystemKeyboardDisabled() const {
  return false;
}

gfx::Rect TextInputClient::GetInputPanelRectangle() const {
  return gfx::Rect();
}

gfx::Rect TextInputClient::GetTextInputBounds() const {
  return gfx::Rect();
}

#if !defined(IS_MAC)
bool TextInputClient::DeleteRange(const gfx::Range& range) {
  return false;
}
#endif
///@}

bool TextInputClient::CanInsertImage() {
  return false;
}

#if BUILDFLAG(IS_CHROMEOS)
void TextInputClient::ExtendSelectionAndReplace(
    size_t length_before_selection,
    size_t length_after_selection,
    const std::u16string_view replacement_string) {
  ExtendSelectionAndDelete(length_before_selection, length_after_selection);
  InsertText(std::u16string(replacement_string),
             InsertTextCursorBehavior::kMoveCursorAfterText);
}

std::optional<GrammarFragment> TextInputClient::GetGrammarFragmentAtCursor()
    const {
  return std::nullopt;
}

bool TextInputClient::ClearGrammarFragments(const gfx::Range& range) {
  return false;
}

bool TextInputClient::AddGrammarFragments(
    const std::vector<GrammarFragment>& fragments) {
  return false;
}

bool TextInputClient::SupportsAlwaysConfirmComposition() {
  return true;
}
#endif

#if BUILDFLAG(IS_WIN) || BUILDFLAG(IS_CHROMEOS)
ui::TextInputClient::EditingContext TextInputClient::GetTextEditingContext() {
  return {};
}
#endif

}  // namespace ui
