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

#include "neva/injection/renderer/injection_events_emitter.h"

#include <algorithm>
#include <iterator>
#include <tuple>
#include <vector>

#include "base/logging.h"
#include "gin/arguments.h"
#include "neva/logging.h"

namespace injections {

const char InjectionEventsEmitterBase::kEmitMethodName[] = "emit";
const char InjectionEventsEmitterBase::kEventNamesMethodName[] = "eventNames";
const char InjectionEventsEmitterBase::kListenerCountMethodName[] =
    "listenerCount";
const char InjectionEventsEmitterBase::kOnMethodName[] = "on";
const char InjectionEventsEmitterBase::kOnceMethodName[] = "once";
const char InjectionEventsEmitterBase::kRemoveEventListenerMethodName[] =
    "removeEventListener";
const char InjectionEventsEmitterBase::kRemoveAllEventListenersMethodName[] =
    "removeAllEventListeners";

InjectionEventsEmitterBase::InjectionEventsEmitterBase() = default;

InjectionEventsEmitterBase::~InjectionEventsEmitterBase() = default;

void InjectionEventsEmitterBase::GetEventNames(gin::Arguments* args) const {
  std::vector<std::string> names;
  std::transform(listeners_.cbegin(), listeners_.cend(),
                 std::back_inserter(names),
                 [](TListenersMap::const_reference ref) { return ref.first; });
  args->Return(gin::ConvertToV8(args->isolate(), names));
}

void InjectionEventsEmitterBase::Emit(gin::Arguments* args) {
  std::string name;
  if (!args->GetNext(&name))
    return;

  auto event_it = listeners_.find(name);
  if (event_it == listeners_.end())
    return;

  auto argv = args->GetAll();
  v8::Isolate* isolate = args->isolate();
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  InvokeAllHandlers(event_it->second, isolate, context, argv.size() - 1,
                    argv.data() + 1);
}

void InjectionEventsEmitterBase::AddEventListener(gin::Arguments* args) {
  DoAddEventListener(args, false);
}

void InjectionEventsEmitterBase::AddOnceEventListener(gin::Arguments* args) {
  DoAddEventListener(args, true);
}

int InjectionEventsEmitterBase::GetListenerCount(
    const std::string& name) const {
  auto it = listeners_.find(name);
  return it == listeners_.cend() ? 0 : it->second.size();
}

void InjectionEventsEmitterBase::RemoveEventListener(gin::Arguments* args) {
  std::string name;
  if (!args->GetNext(&name))
    return;

  v8::Local<v8::Function> func;
  if (!args->GetNext(&func))
    return;

  auto event_it = listeners_.find(name);
  if (event_it == listeners_.end())
    return;

  auto& handlers = event_it->second;

  // If listener has been added several times, the most recently added listener
  // will be removed.
  auto func_it = std::find_if(
      handlers.rbegin(), handlers.rend(),
      [&func](TFuncList::const_reference val) { return val.first == func; });

  if (func_it != handlers.rend())
    handlers.erase(std::prev(func_it.base()));
}

void InjectionEventsEmitterBase::RemoveAllEventListeners(gin::Arguments* args) {
  if (args->Length() == 0) {
    listeners_.clear();
    return;
  }

  std::string name;
  if (args->GetNext(&name))
    std::ignore = listeners_.erase(name);
}

bool InjectionEventsEmitterBase::HasEventListener(
    const std::string& event_name) {
  return listeners_.find(event_name) != listeners_.end();
}

void InjectionEventsEmitterBase::DoAddEventListener(gin::Arguments* args,
                                                    bool is_once_listener) {
  std::string name;
  if (!args->GetNext(&name))
    return;

  v8::Local<v8::Function> func;
  if (!args->GetNext(&func))
    return;

  TListenersMap::iterator it;
  std::tie(it, std::ignore) = listeners_.insert({std::move(name), TFuncList()});
  v8::Global<v8::Function> f_glob(args->isolate(), func);
  it->second.push_back({std::move(f_glob), std::move(is_once_listener)});
}

void InjectionEventsEmitterBase::InvokeAllHandlers(
    TFuncList& handlers,
    v8::Isolate* isolate,
    v8::Local<v8::Context>& context,
    int argc,
    v8::Local<v8::Value> argv[]) {

  std::list<v8::Local<v8::Function>> pending;
  for (auto it = handlers.begin(); it != handlers.end();) {
    pending.push_back(it->first.Get(isolate));
    it = it->second ? handlers.erase(it) : std::next(it);
  }

  for (auto& func : pending)
    std::ignore = func->Call(context, v8::Undefined(isolate), argc, argv);
}

}  // namespace injections
