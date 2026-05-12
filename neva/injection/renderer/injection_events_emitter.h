// Copyright 2021 LG Electronics, Inc.
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

#ifndef NEVA_INJECTION_RENDERER_INJECTION_EVENTS_EMITTER_H_
#define NEVA_INJECTION_RENDERER_INJECTION_EVENTS_EMITTER_H_

#include <list>
#include <string>
#include <unordered_map>

#include "gin/converter.h"
#include "v8/include/v8.h"

namespace gin {
class Arguments;
}

namespace injections {

class InjectionEventsEmitterBase {
 public:
  static const char kEmitMethodName[];
  static const char kEventNamesMethodName[];
  static const char kListenerCountMethodName[];
  static const char kOnMethodName[];
  static const char kOnceMethodName[];
  static const char kRemoveEventListenerMethodName[];
  static const char kRemoveAllEventListenersMethodName[];

  InjectionEventsEmitterBase();
  InjectionEventsEmitterBase(const InjectionEventsEmitterBase&) = delete;
  InjectionEventsEmitterBase& operator=(const InjectionEventsEmitterBase&) =
      delete;
  virtual ~InjectionEventsEmitterBase();

  void GetEventNames(gin::Arguments* args) const;
  void Emit(gin::Arguments* args);
  void AddEventListener(gin::Arguments* args);
  void AddOnceEventListener(gin::Arguments* args);
  int GetListenerCount(const std::string& name) const;
  void RemoveEventListener(gin::Arguments* args);
  void RemoveAllEventListeners(gin::Arguments* args);
  bool HasEventListener(const std::string& event_name);

 private:
  void DoAddEventListener(gin::Arguments* args, bool is_once_listener);

 protected:
  using TFuncList = std::list<std::pair<v8::Global<v8::Function>, bool>>;
  using TListenersMap = std::unordered_map<std::string, TFuncList>;

  void InvokeAllHandlers(TFuncList& handlers,
                         v8::Isolate* isolate,
                         v8::Local<v8::Context>& context,
                         int argc,
                         v8::Local<v8::Value> argv[]);
  TListenersMap listeners_;
};

template <typename Derived>
class InjectionEventsEmitter : public InjectionEventsEmitterBase {
 public:
  template <typename... Ts>
  void DoEmit(const std::string& event, Ts... args) {
    auto it = listeners_.find(event);
    if (it != listeners_.end())
      InvokeInJS(it->second, args...);
  }

 private:
  template <typename... Ts>
  void InvokeInJS(TFuncList& handlers, Ts... args) {
    auto* self = static_cast<Derived*>(this);
    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    v8::HandleScope handle_scope(isolate);
    v8::MaybeLocal<v8::Object> maybe_wrapper = self->GetWrapper(isolate);
    v8::Local<v8::Object> wrapper;
    if (maybe_wrapper.ToLocal(&wrapper)) {
      v8::Local<v8::Context> context;
      if (wrapper->GetCreationContext().ToLocal(&context)) {
        v8::MicrotasksScope microtasksScope(
            context, v8::MicrotasksScope::kRunMicrotasks);
        v8::Context::Scope context_scope(context);
        std::vector<v8::Local<v8::Value>> argv;
        UnwrapAndPassArgs(handlers, isolate, context, argv, args...);
      }
    }
  }

  template <typename TFirst, typename... Ts>
  void UnwrapAndPassArgs(TFuncList& handlers,
                         v8::Isolate* isolate,
                         v8::Local<v8::Context>& context,
                         std::vector<v8::Local<v8::Value>>& argv,
                         TFirst val,
                         Ts... args) {
    argv.push_back(gin::Converter<TFirst>::ToV8(isolate, val));
    UnwrapAndPassArgs(handlers, isolate, context, argv, args...);
  }

  void UnwrapAndPassArgs(TFuncList& handlers,
                         v8::Isolate* isolate,
                         v8::Local<v8::Context>& context,
                         std::vector<v8::Local<v8::Value>>& argv) {
    InvokeAllHandlers(handlers, isolate, context, argv.size(), argv.data());
  }
};

}  // namespace injections

#endif  // NEVA_INJECTION_RENDERER_INJECTION_EVENTS_EMITTER_H_
