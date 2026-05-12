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

#ifndef NEVA_INJECTION_RENDERER_WEBOSYSTSEM_WINDOW_INJECTION_H_
#define NEVA_INJECTION_RENDERER_WEBOSYSTSEM_WINDOW_INJECTION_H_

#include <string>

#include "gin/object_template_builder.h"
#include "gin/wrappable.h"
#include "v8/include/v8.h"

namespace gin {

class Arguments;

}  // namespace gin

namespace injections {

class WindowInjection : public gin::Wrappable<WindowInjection> {
 public:
  static gin::WrapperInfo kWrapperInfo;

  class Delegate {
   public:
    virtual void SetInputRegion(gin::Arguments* args) = 0;
    virtual void SetWindowProperty(const std::string& arg1,
                                   const std::string& arg2) = 0;
  };

  explicit WindowInjection(Delegate* delegate);
  WindowInjection(const WindowInjection&) = delete;
  WindowInjection& operator=(const WindowInjection&) = delete;
  ~WindowInjection() override;

 private:
  // gin::Wrappable.
  gin::ObjectTemplateBuilder GetObjectTemplateBuilder(
      v8::Isolate* isolate) final;

  void SetInputRegion(gin::Arguments* args);
  void SetProperty(const std::string& arg1, const std::string& arg2);

  Delegate* delegate_;
};

}  // namespace injections

#endif  // NEVA_INJECTION_RENDERER_WEBOSYSTSEM_WINDOW_INJECTION_H_
