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

#include "neva/injection/renderer/webossystem/cursor_injection.h"

#include "gin/arguments.h"

namespace injections {

gin::WrapperInfo CursorInjection::kWrapperInfo = {gin::kEmbedderNativeGin};

CursorInjection::CursorInjection(Delegate* delegate) : delegate_(delegate) {}

CursorInjection::~CursorInjection() = default;

gin::ObjectTemplateBuilder CursorInjection::GetObjectTemplateBuilder(
    v8::Isolate* isolate) {
  return gin::Wrappable<CursorInjection>::GetObjectTemplateBuilder(isolate)
      .SetProperty("visibility", &CursorInjection::GetCursorVisibility)
      .SetMethod("getCursorState", &CursorInjection::GetCursorState)
      .SetMethod("setCursor", &CursorInjection::SetCursor)
      .SetMethod("hide", &CursorInjection::HideCursor);
}

bool CursorInjection::GetCursorVisibility() {
  return delegate_->CallFunctionName("cursorVisibility") == "true";
}

std::string CursorInjection::GetCursorState() {
  return "{ \"visibility\" : " +
         delegate_->CallFunctionName("cursorVisibility") + " }";
}

bool CursorInjection::HideCursor() {
  return delegate_->HideCursor();
}

bool CursorInjection::SetCursor(gin::Arguments* args) {
  return delegate_->SetCursor(args);
}

}  // namespace injections
