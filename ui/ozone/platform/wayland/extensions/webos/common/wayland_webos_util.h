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

#ifndef UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_COMMON_WAYLAND_WEBOS_UTIL_H_
#define UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_COMMON_WAYLAND_WEBOS_UTIL_H_

#include <type_traits>

namespace ui {

namespace {

// underlying type alias
template <typename E,
          typename = typename std::enable_if_t<std::is_enum<E>::value>>
using UnderlyingType = typename std::underlying_type<E>::type;

// <enum class constant> | <enum class constant> operator
// returning a <value of underlying type>
template <typename E>
typename std::enable_if_t<std::is_enum<E>::value, UnderlyingType<E>>
operator|(E lhs, E rhs) {
  return static_cast<UnderlyingType<E>>(lhs) |
         static_cast<UnderlyingType<E>>(rhs);
}

// <variable of underlying type> |= <enum class constant> operator
// returning the <variable of underlying type>
template <typename E>
typename std::enable_if_t<std::is_enum<E>::value, UnderlyingType<E>&>
operator|=(UnderlyingType<E>& lhs, E rhs) {
  lhs =
      static_cast<UnderlyingType<E>>(lhs) | static_cast<UnderlyingType<E>>(rhs);
  return lhs;
}

// <value of underlying type> & <constant of underlying type> operator
// returning a <value of underlying type>
template <typename E>
typename std::enable_if_t<std::is_enum<E>::value, UnderlyingType<E>>
operator&(UnderlyingType<E> lhs, E rhs) {
  return lhs & static_cast<UnderlyingType<E>>(rhs);
}

}  // namespace

}  // namespace ui

#endif  // UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_COMMON_WAYLAND_WEBOS_UTIL_H_
