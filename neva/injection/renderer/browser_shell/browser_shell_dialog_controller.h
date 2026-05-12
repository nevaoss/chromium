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

#ifndef NEVA_INJECTION_RENDERER_BROWSER_SHELL_BROWSER_SHELL_DIALOG_CONTROLLER_H_
#define NEVA_INJECTION_RENDERER_BROWSER_SHELL_BROWSER_SHELL_DIALOG_CONTROLLER_H_

#include <string>

#include "base/memory/raw_ptr.h"
#include "gin/object_template_builder.h"
#include "gin/wrappable.h"
#include "neva/browser_shell/service/public/mojom/browser_shell_page_contents.mojom.h"

namespace gin {
class Arguments;
}  // namespace gin

namespace injections {

class DialogController : public gin::Wrappable<DialogController> {
 public:
  static gin::WrapperInfo kWrapperInfo;

  class Delegate {
   public:
    virtual void CloseJSDialog(bool success, const std::string& response) = 0;
  };

  explicit DialogController(Delegate* delegate);
  DialogController(const DialogController&) = delete;
  DialogController& operator=(const DialogController&) = delete;
  ~DialogController() override;

 private:
  gin::ObjectTemplateBuilder GetObjectTemplateBuilder(
      v8::Isolate* isolate) final;

  void Ok(gin::Arguments* args);
  void Cancel();

  raw_ptr<Delegate> delegate_;
};

}  // namespace injections

#endif  // NEVA_INJECTION_RENDERER_BROWSER_SHELL_BROWSER_SHELL_DIALOG_CONTROLLER_H_
