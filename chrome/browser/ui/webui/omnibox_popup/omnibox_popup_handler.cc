// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/omnibox_popup/omnibox_popup_handler.h"
#include "ui/base/models/menu_model.h"

OmniboxPopupHandler::OmniboxPopupHandler(
    mojo::PendingReceiver<omnibox_popup::mojom::PageHandler> receiver,
    mojo::PendingRemote<omnibox_popup::mojom::Page> page,
    content::WebContents* web_contents)
    : receiver_(this, std::move(receiver)),
      page_(std::move(page)),
      web_contents_(web_contents) {}

OmniboxPopupHandler::~OmniboxPopupHandler() = default;

void OmniboxPopupHandler::ShowContextMenu(const gfx::Point& point) {
  if (embedder_) {
    embedder_->ShowContextMenu(point, nullptr);
  }
}

void OmniboxPopupHandler::CloseUI() {
  if (embedder_) {
    embedder_->CloseUI();
  }
}

void OmniboxPopupHandler::OnSelectionChanged(
    omnibox_popup::mojom::OmniboxInputStatePtr state) {
  if (state->sequence_number < current_sequence_number_) {
    return;
  }
  latest_selection_ = state->selection;
}

void OmniboxPopupHandler::OnShow() {
  page_->OnShow();
}

void OmniboxPopupHandler::OnContextMenuClosed() {
  page_->OnContextMenuClosed();
}

void OmniboxPopupHandler::SetInputState(const std::string& text,
                                        const gfx::Range& selection) {
  latest_selection_ = selection;
  current_sequence_number_++;
  auto state = omnibox_popup::mojom::OmniboxInputState::New();
  state->text = text;
  state->selection = selection;
  state->sequence_number = current_sequence_number_;
  page_->SetInputState(std::move(state));
}
