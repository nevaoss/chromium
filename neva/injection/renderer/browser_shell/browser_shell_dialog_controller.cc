// Copyright 2022 LG Electronics, Inc.
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

#include "neva/injection/renderer/browser_shell/browser_shell_dialog_controller.h"

#include "base/logging.h"
#include "gin/arguments.h"

namespace injections {

DialogController::DialogController(Delegate* delegate) : delegate_(delegate) {}

DialogController::~DialogController() {
  if (delegate_) {
    LOG(ERROR) << "'dialog' event handler has not been complete correctly."
               << "Either the Ok() or the Cancel() method must be called.";
    delegate_->CloseJSDialog(false, std::string());
  }
}

gin::ObjectTemplateBuilder DialogController::GetObjectTemplateBuilder(
    v8::Isolate* isolate) {
  return gin::Wrappable<DialogController>::GetObjectTemplateBuilder(isolate)
      .SetMethod("ok", &DialogController::Ok)
      .SetMethod("cancel", &DialogController::Cancel);
}

void DialogController::Ok(gin::Arguments* args) {
  if (!delegate_)
    return;

  std::string response;
  std::ignore = args->GetNext(&response);
  delegate_->CloseJSDialog(true, response);
  delegate_ = nullptr;
}

void DialogController::Cancel() {
  if (!delegate_)
    return;

  delegate_->CloseJSDialog(false, std::string());
  delegate_ = nullptr;
}

const gin::WrapperInfo* DialogController::wrapper_info() const {
  return &kWrapperInfo;
}

}  // namespace injections
