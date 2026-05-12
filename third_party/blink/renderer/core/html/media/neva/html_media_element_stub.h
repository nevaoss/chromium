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

#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_HTML_MEDIA_NEVA_HTML_MEDIA_ELEMENT_STUB_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_HTML_MEDIA_NEVA_HTML_MEDIA_ELEMENT_STUB_H_

#include <stdio.h>

#include <limits>
#include <type_traits>

#include "third_party/blink/renderer/bindings/core/v8/to_v8_traits.h"

namespace blink {
namespace neva {

template <typename original_t>
class HTMLMediaElement {
 public:
  HTMLMediaElement();

  ScriptValue getStartDate(ScriptState* script_state) const;
  const String mediaId() const;

  // Neva audio focus extensions
  bool webosMediaFocus() const;
  void setWebosMediaFocus(bool focus);
};

template <typename original_t>
HTMLMediaElement<original_t>::HTMLMediaElement() {}

template <typename original_t>
ScriptValue HTMLMediaElement<original_t>::getStartDate(
    ScriptState* script_state) const {
  base::Time time = base::Time::FromMillisecondsSinceUnixEpoch(
      std::numeric_limits<double>::quiet_NaN());
  return ScriptValue(script_state->GetIsolate(),
                     ToV8Traits<IDLNullable<IDLDate>>::ToV8(
                         script_state, absl::optional<base::Time>(time)));
}

template <typename original_t>
const String HTMLMediaElement<original_t>::mediaId() const {
  return String();
}

template <typename original_t>
bool HTMLMediaElement<original_t>::webosMediaFocus() const {
  return false;
}

template <typename original_t>
void HTMLMediaElement<original_t>::setWebosMediaFocus(bool focus) {}

}  // namespace neva
}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_CORE_HTML_MEDIA_NEVA_HTML_MEDIA_ELEMENT_STUB_H_
