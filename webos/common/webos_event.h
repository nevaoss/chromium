// Copyright 2016-2019 LG Electronics, Inc.
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

#ifndef WEBOS_COMMON_WEBOS_EVENT_H_
#define WEBOS_COMMON_WEBOS_EVENT_H_

#include "neva/app_runtime/public/app_runtime_event.h"
#include "webos/common/webos_export.h"

class WEBOS_EXPORT WebOSEvent : public neva_app_runtime::AppRuntimeEvent {
 public:
  explicit WebOSEvent(neva_app_runtime::AppRuntimeEvent::Type type, int flags = 0)
      : neva_app_runtime::AppRuntimeEvent(type, flags) {}
  ~WebOSEvent() override {}
};

class WEBOS_EXPORT WebOSKeyEvent : public WebOSEvent,
                                   public neva_app_runtime::KeyEvent {
 public:
  explicit WebOSKeyEvent(neva_app_runtime::AppRuntimeKeyEvent::Type type,
                         unsigned code)
      : WebOSEvent(type), neva_app_runtime::KeyEvent(code) {}
  void SetCode(unsigned code) { code_ = code; }
  void SetDomKey(int32_t dom_key) { dom_key_ = dom_key; }
  ~WebOSKeyEvent() override {}
};

class WEBOS_EXPORT WebOSMouseEvent
    : public WebOSEvent,
      public neva_app_runtime::MouseEvent {
 public:
  using Button = neva_app_runtime::MouseEvent::Button;

  explicit WebOSMouseEvent(neva_app_runtime::AppRuntimeKeyEvent::Type type,
                           float x,
                           float y,
                           int flags = 0)
      : WebOSEvent(type, flags), neva_app_runtime::MouseEvent(x, y) {}
  ~WebOSMouseEvent() override {}

  int GetFlags() override { return AppRuntimeEvent::GetFlags(); }
};

class WEBOS_EXPORT WebOSMouseWheelEvent
    : public WebOSMouseEvent,
      public neva_app_runtime::MouseWheelEvent {
 public:
  explicit WebOSMouseWheelEvent(neva_app_runtime::AppRuntimeKeyEvent::Type type,
                                float x,
                                float y,
                                float x_offset,
                                float y_offset)
      : WebOSMouseEvent(type, x, y),
        neva_app_runtime::MouseWheelEvent(x_offset, y_offset) {}
  ~WebOSMouseWheelEvent() override {}
};

class WEBOS_EXPORT WebOSVirtualKeyboardEvent
   : public WebOSEvent,
     public neva_app_runtime::VirtualKeyboardEvent {
 public:
  explicit WebOSVirtualKeyboardEvent(neva_app_runtime::AppRuntimeKeyEvent::Type type,
                                     bool visible,
                                     int height,
                                     int flags = 0)
      : WebOSEvent(type, flags),
        neva_app_runtime::VirtualKeyboardEvent(visible, height) {}
  ~WebOSVirtualKeyboardEvent() override {}
};

#endif  // WEBOS_COMMON_WEBOS_EVENT_H_
