// Copyright 2020 LG Electronics, Inc.
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

#include "ui/ozone/platform/wayland/extensions/webos/host/webos_input_panel.h"

#include "ui/ozone/platform/wayland/extensions/webos/host/wayland_extensions_webos.h"
#include "ui/ozone/platform/wayland/extensions/webos/host/wayland_window_webos.h"
#include "ui/ozone/platform/wayland/extensions/webos/host/webos_text_model_factory_wrapper.h"
#include "ui/ozone/platform/wayland/extensions/webos/host/webos_text_model_wrapper.h"
#include "ui/ozone/platform/wayland/host/wayland_connection.h"

namespace ui {

WebosInputPanel::WebosInputPanel(WaylandConnection* connection,
                                 WaylandWindowWebos* window)
    : connection_(connection), window_(window) {}

WebosInputPanel::~WebosInputPanel() = default;

void WebosInputPanel::HideInputPanel() {
  if (!webos_text_model_)
    return;

  webos_text_model_->Reset();
  webos_text_model_->HideInputPanel();
  webos_text_model_->Deactivate();
  webos_text_model_.reset();

  if (window_) {
    window_->OnInputPanelVisibilityChanged(false);
    window_->HandleInputPanelRectangleChange(0, 0, 0, 0);
  }
}

void WebosInputPanel::SetTextInputInfo(const TextInputInfo& text_input_info) {
  // The below attributes need to be stored to make deferred setting of the
  // input content type possible (e.g., if text model is not yet activated).
  input_content_type_ = text_input_info.type;
  input_flags_ = text_input_info.flags;
  input_panel_rect_ = text_input_info.input_panel_rectangle;

  if (webos_text_model_ && webos_text_model_->IsActivated())
    UpdateTextModel();
}

void WebosInputPanel::SetSurroundingText(const std::string& text,
                                         std::size_t cursor_position,
                                         std::size_t anchor_position) {
  if (webos_text_model_)
    webos_text_model_->SetSurroundingText(text, cursor_position,
                                          anchor_position);
}

void WebosInputPanel::ShowInputPanel() {
  if (!webos_text_model_ && !CreateTextModel())
    return;

  if (webos_text_model_->IsActivated()) {
    webos_text_model_->ShowInputPanel();
  } else {
    webos_text_model_->Activate();
    UpdateTextModel();
  }
}

bool WebosInputPanel::CreateTextModel() {
  if (webos_text_model_)
    return false;

  WaylandExtensionsWebos* webos_extensions = window_->GetWebosExtensions();
  WebosTextModelFactoryWrapper* webos_text_model_factory =
      webos_extensions->GetWebosTextModelFactory();
  if (webos_text_model_factory) {
    webos_text_model_.reset(webos_text_model_factory->
        CreateTextModel(this, connection_, window_).release());
  }

  return !!webos_text_model_;
}

void WebosInputPanel::UpdateTextModel() {
  if (webos_text_model_) {
    if (input_panel_rect_.IsEmpty())
      webos_text_model_->ResetInputPanelRect();
    else
      webos_text_model_->SetInputPanelRect(
          input_panel_rect_.x(), input_panel_rect_.y(),
          input_panel_rect_.width(), input_panel_rect_.height());
    webos_text_model_->SetContentType(input_content_type_, input_flags_);
  }
}

void WebosInputPanel::SetInputPanelRect(const gfx::Rect& input_panel_rect) {
  input_panel_rect_ = input_panel_rect;
}

gfx::Rect WebosInputPanel::GetInputPanelRect() {
  if (!webos_text_model_ || !webos_text_model_->IsActivated())
    return gfx::Rect();
  return input_panel_rect_;
}

}  // namespace ui
