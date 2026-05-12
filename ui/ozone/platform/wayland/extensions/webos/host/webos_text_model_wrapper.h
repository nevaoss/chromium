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
//

#ifndef UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WEBOS_TEXT_MODEL_WRAPPER_H_
#define UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WEBOS_TEXT_MODEL_WRAPPER_H_

#include <cstdint>
#include <string>

#include "ui/base/ime/neva/input_content_type.h"
#include "ui/ozone/platform/wayland/extensions/webos/common/wayland_webos_object.h"

namespace ui {

class WaylandInputMethodContext;
class WaylandSeat;
class WaylandWindowWebos;
class WebosInputPanel;

// Wrapper for the Wayland |text_model| interface used to manage text input
// and input method for an application.
class WebosTextModelWrapper {
 public:
  WebosTextModelWrapper(text_model* model,
                        WebosInputPanel* input_panel,
                        WaylandSeat* seat,
                        WaylandWindowWebos* window);
  WebosTextModelWrapper(const WebosTextModelWrapper&) = delete;
  WebosTextModelWrapper& operator=(const WebosTextModelWrapper&) = delete;
  ~WebosTextModelWrapper();

  // Sets a surrounding text around the input position.
  void SetSurroundingText(const std::string& text,
                          std::uint32_t cursor,
                          std::uint32_t anchor);

  // Activates the text model.
  void Activate();

  // Deactivates the text model.
  void Deactivate();

  // Resets the input state.
  void Reset();

  // Sets the cursor rectangle (in surface coordinates).
  void SetCursorRectangle(std::int32_t x,
                          std::int32_t y,
                          std::int32_t width,
                          std::int32_t height);

  // Sets content purpose and hint to the input panel.
  void SetContentType(InputContentType type, int flags);

  // Invokes a button action at a given position (|index|).
  void InvokeAction(std::uint32_t button, std::uint32_t index);

  // Commits pending input state changes.
  void Commit();

  // Shows the input panel (virtual keyboard).
  void ShowInputPanel();

  // Hides the input panel (virtual keyboard).
  void HideInputPanel();

  // Sets the maximum |length| for an input field.
  void SetMaxTextLength(std::uint32_t length);

  // Sets a platform specific data (|text|).
  void SetPlatformData(const std::string& text);

  // Sets type of the enter key.
  void SetEnterKeyType(std::uint32_t enter_key_type);

  // Changes geometry of the input panel.
  void SetInputPanelRect(std::int32_t x,
                         std::int32_t y,
                         std::uint32_t width,
                         std::uint32_t height);

  // Resets geometry of the input panel.
  void ResetInputPanelRect();

  // text_model_listener
  // Notifies when the |text| is to be inserted into the editor widget.
  static void CommitString(void* data,
                           text_model* text_model,
                           std::uint32_t serial,
                           const char* text);
  // Notifies when the pre-edit |text| is to be set around the current cursor.
  static void PreeditString(void* data,
                            text_model* text_model,
                            std::uint32_t serial,
                            const char* text,
                            const char* commit);
  // Notifies when a text around the current cursor is to be deleted.
  static void DeleteSurroundingText(void* data,
                                    text_model* text_model,
                                    std::uint32_t serial,
                                    std::int32_t index,
                                    std::uint32_t length);
  // Notifies when cursor or anchor position is to be modified.
  static void CursorPosition(void* data,
                             text_model* text_model,
                             std::uint32_t serial,
                             std::int32_t index,
                             std::int32_t anchor);
  // Notifies when styling information is to be applied onto a composed text.
  static void PreeditStyling(void* data,
                             text_model* text_model,
                             std::uint32_t serial,
                             std::uint32_t index,
                             std::uint32_t length,
                             std::uint32_t style);
  // Notifies when a cursor position inside a composed text is to be set.
  static void PreeditCursor(void* data,
                            text_model* text_model,
                            std::uint32_t serial,
                            std::int32_t index);
  // Notifies when an array of null-terminated modifier names is sent.
  static void ModifiersMap(void* data, text_model* text_model, wl_array* map);
  // Notifies when a key event is sent.
  static void Keysym(void* data,
                     text_model* text_model,
                     std::uint32_t serial,
                     std::uint32_t time,
                     std::uint32_t sym,
                     std::uint32_t state,
                     std::uint32_t modifiers);
  // Notifies when the |model| is activated on a given surface.
  static void Enter(void* data, text_model* text_model, wl_surface* surface);
  // Notifies when the |model| is deactivated.
  static void Leave(void* data, text_model* model);
  // Notifies when the visibility |state| is changed.
  static void InputPanelState(void* data,
                              text_model* text_model,
                              std::uint32_t state);
  // Notifies when geometry of the input panel is changed.
  static void InputPanelRect(void* data,
                             text_model* text_model,
                             std::int32_t x,
                             std::int32_t y,
                             std::uint32_t width,
                             std::uint32_t height);

  bool IsActivated() const { return is_activated_; }

 private:
  wl::Object<text_model> text_model_;

  WebosInputPanel* const input_panel_;
  WaylandInputMethodContext* const input_method_context_;
  WaylandSeat* const seat_;
  WaylandWindowWebos* const window_;
  enum class TextModelState { kHidden, kShown };
  TextModelState state_ = TextModelState::kHidden;

  bool is_activated_ = false;
  std::int32_t preedit_cursor_ = 0;
};

}  // namespace ui

#endif  // UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_HOST_WEBOS_TEXT_MODEL_WRAPPER_H_
