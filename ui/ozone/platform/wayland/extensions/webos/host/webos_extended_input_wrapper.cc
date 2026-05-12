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

#include "ui/ozone/platform/wayland/extensions/webos/host/webos_extended_input_wrapper.h"

#include <wayland-webos-extension-client-protocol.h>

#include "base/notreached.h"

namespace ui {

namespace {

std::uint32_t KeySymbolTypeFromUiKeySymbolType(
    XInputKeySymbolType symbol_type) {
  wl_webos_xinput_keysym_type result = WL_WEBOS_XINPUT_KEYSYM_TYPE_QT;
  switch (symbol_type) {
    case XINPUT_QT_KEY_SYMBOL:
      result = WL_WEBOS_XINPUT_KEYSYM_TYPE_QT;
      break;
    case XINPUT_NATIVE_KEY_SYMBOL:
      result = WL_WEBOS_XINPUT_KEYSYM_TYPE_NATIVE;
      break;
    default:
      NOTREACHED();
      break;
  }
  return static_cast<std::uint32_t>(result);
}

std::uint32_t EventTypeFromUiEventType(XInputEventType event_type) {
  wl_webos_xinput_event_type result =
      WL_WEBOS_XINPUT_EVENT_TYPE_PRESS_AND_RELEASE;
  switch (event_type) {
    case XINPUT_PRESS_AND_RELEASE:
      result = WL_WEBOS_XINPUT_EVENT_TYPE_PRESS_AND_RELEASE;
      break;
    case XINPUT_PRESS:
      result = WL_WEBOS_XINPUT_EVENT_TYPE_PRESS;
      break;
    case XINPUT_RELEASE:
      result = WL_WEBOS_XINPUT_EVENT_TYPE_RELEASE;
      break;
    default:
      NOTREACHED();
      break;
  }
  return static_cast<std::uint32_t>(result);
}

}  // namespace

WebosExtendedInputWrapper::WebosExtendedInputWrapper(wl_webos_xinput* xinput) {
  webos_xinput_.reset(xinput);

  static const wl_webos_xinput_listener webos_xinput_listener = {
      WebosExtendedInputWrapper::Activated,
      WebosExtendedInputWrapper::Deactivated};

  wl_webos_xinput_add_listener(webos_xinput_.get(), &webos_xinput_listener,
                               this);
}

WebosExtendedInputWrapper::~WebosExtendedInputWrapper() = default;

void WebosExtendedInputWrapper::Activate(const std::string& type) {
  wl_webos_xinput_activated(webos_xinput_.get(), type.c_str());
}

void WebosExtendedInputWrapper::Deactivate() {
  wl_webos_xinput_deactivated(webos_xinput_.get());
}

void WebosExtendedInputWrapper::InvokeAction(std::uint32_t keysym,
                                             XInputKeySymbolType symbol_type,
                                             XInputEventType event_type) {
  wl_webos_xinput_invoke_action(webos_xinput_.get(), keysym,
                                KeySymbolTypeFromUiKeySymbolType(symbol_type),
                                EventTypeFromUiEventType(event_type));
}

// static
void WebosExtendedInputWrapper::Activated(void* data,
                                          wl_webos_xinput* wl_webos_xinput,
                                          const char* type) {
  NOTIMPLEMENTED_LOG_ONCE();
}

void WebosExtendedInputWrapper::Deactivated(void* data,
                                            wl_webos_xinput* wl_webos_xinput) {
  NOTIMPLEMENTED_LOG_ONCE();
}

}  // namespace ui
