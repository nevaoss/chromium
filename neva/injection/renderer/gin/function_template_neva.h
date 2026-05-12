// Copyright 2018 LG Electronics, Inc.
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

#ifndef NEVA_INJECTION_COMMON_GIN_FUNCTION_TEMPLATE_NEVA_H_
#define NEVA_INJECTION_COMMON_GIN_FUNCTION_TEMPLATE_NEVA_H_

#include "gin/function_template.h"

namespace gin {

template <typename Sig>
v8::Local<v8::FunctionTemplate> CreateConstructorTemplate(
    v8::Isolate* isolate,
    base::RepeatingCallback<Sig> callback,
    InvokerOptions invoker_options = {}) {
  typedef internal::CallbackHolder<Sig> HolderT;
  HolderT* holder =
      new HolderT(isolate, std::move(callback), std::move(invoker_options));

  v8::Local<v8::FunctionTemplate> tmpl = v8::FunctionTemplate::New(
      isolate, &internal::Dispatcher<Sig>::DispatchToCallback,
      ConvertToV8<v8::Local<v8::External>>(isolate,
                                           holder->GetHandle(isolate)));
  return tmpl;
}

}  // gin

#endif  // NEVA_INJECTION_COMMON_GIN_FUNCTION_TEMPLATE_NEVA_H_
