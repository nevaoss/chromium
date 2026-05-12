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

#include "neva/injection/renderer/webossystem/window_injection.h"

#include "gin/arguments.h"

namespace injections {

gin::WrapperInfo WindowInjection::kWrapperInfo = {gin::kEmbedderNativeGin};

WindowInjection::WindowInjection(Delegate* delegate) : delegate_(delegate) {}

WindowInjection::~WindowInjection() = default;

gin::ObjectTemplateBuilder WindowInjection::GetObjectTemplateBuilder(
    v8::Isolate* isolate) {
  return gin::Wrappable<WindowInjection>::GetObjectTemplateBuilder(isolate)
      .SetMethod("setInputRegion", &WindowInjection::SetInputRegion)
      .SetMethod("setProperty", &WindowInjection::SetProperty);
}

void WindowInjection::SetInputRegion(gin::Arguments* args) {
  delegate_->SetInputRegion(args);
}

void WindowInjection::SetProperty(const std::string& arg1,
                                  const std::string& arg2) {
  delegate_->SetWindowProperty(arg1, arg2);
}

}  // namespace injections
