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

#ifndef NEVA_INJECTION_RENDERER_WEBOSYSTSEM_CURSOR_INJECTION_H_
#define NEVA_INJECTION_RENDERER_WEBOSYSTSEM_CURSOR_INJECTION_H_

#include <string>

#include "gin/object_template_builder.h"
#include "gin/wrappable.h"
#include "v8/include/v8.h"

namespace gin {

class Arguments;

}  // namespace gin

namespace injections {

class CursorInjection : public gin::Wrappable<CursorInjection> {
 public:
  static gin::WrapperInfo kWrapperInfo;

  class Delegate {
   public:
    virtual std::string CallFunctionName(const std::string& name) = 0;
    virtual bool HideCursor() = 0;
    virtual bool SetCursor(gin::Arguments* args) = 0;
  };

  explicit CursorInjection(Delegate* delegate);
  CursorInjection(const CursorInjection&) = delete;
  CursorInjection& operator=(const CursorInjection&) = delete;
  ~CursorInjection() override;

 private:
  // gin::Wrappable.
  gin::ObjectTemplateBuilder GetObjectTemplateBuilder(
      v8::Isolate* isolate) final;

  bool GetCursorVisibility();
  bool HideCursor();
  bool SetCursor(gin::Arguments* args);
  std::string GetCursorState();

  Delegate* delegate_;
};

}  // namespace injections

#endif  // NEVA_INJECTION_RENDERER_WEBOSYSTSEM_CURSOR_INJECTION_H_
