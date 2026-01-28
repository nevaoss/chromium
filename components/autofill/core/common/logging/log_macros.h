// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_AUTOFILL_CORE_COMMON_LOGGING_LOG_MACROS_H_
#define COMPONENTS_AUTOFILL_CORE_COMMON_LOGGING_LOG_MACROS_H_

// Logging macro in the style of LOG(INFO) intended for
// chrome://autofill-internals.
//
// In `LOG_AF(logger) << expression`, the `expression` is evaluated only
// if the `logger` is active. The expression `logger` must be of type
// `LogManager` or `LogBuffer` or `LogManager*` or `LogBuffer`*.
//
// Support for other types of `logger` can be added by adding template
// specializations of `LoggerTraits`.
//
// This macro works as follows:
//   LOG_AF(logger) << foo();
// expands to
//   !active(logger) ? (void)0 : Voidify() & get_stream(logger) << foo();
// Due to the operator precedence, this is equivalent to:
//   !active(logger) ? (void)0 : (Voidify() & (get_stream(logger) << foo()));
// If the logger is inactive, this is equivalent to the no-op
//   (void)0;
// and otherwise it is equivalent to
//   get_stream(logger) << foo();
#define LOG_AF(logger)                                                        \
  !::autofill::internal::LoggerTraits<decltype(logger)>::active(logger)       \
      ? (void)0                                                               \
      : ::autofill::internal::Voidify() &                                     \
            ::autofill::internal::LoggerTraits<decltype(logger)>::get_stream( \
                logger)

namespace autofill::internal {

// TODO(neva): Remove this when Neva GCC supports static_assert(false).
#if defined(__GNUC__) && (__GNUC__ < 13) && !defined(__clang__)
// Workaround for compile-time assertions involving dependent types
template<typename>
constexpr bool dependent_false = false; // Always false
#endif  // defined(__GNUC__) && (__GNUC__ < 13) && !defined(__clang__)

// Traits for targets of LOG_AF(). There are currently specializations for
// `LogManager*` and `LogBuffer*`. The below is just a placeholder.
template <typename T>
struct LoggerTraits {
  class Null {
   public:
    constexpr Null() = default;
    template <typename U>
    Null operator<<(const U&) {
      return {};
    }
  };

  // Returns true iff logging to should be enabled.
  static bool active(const T& logger) {
// TODO(neva): Remove this when Neva GCC supports static_assert(false).
#if defined(__GNUC__) && (__GNUC__ < 13) && !defined(__clang__)
    static_assert(
        dependent_false<T>,
#else   // defined(__GNUC__) && (__GNUC__ < 13) && !defined(__clang__)
    static_assert(
        false,
#endif  // !(defined(__GNUC__) && (__GNUC__ < 13) && !defined(__clang__))
        "Traits for this logger aren't visible. Are you missing an include of "
        "log_manager.h and/or log_buffer.h?");
    return false;
  }

  // Returns an object that implements the stream insertion operator
  // operator<<().
  static Null get_stream(const T& logger) {
// TODO(neva): Remove this when Neva GCC supports static_assert(false).
#if defined(__GNUC__) && (__GNUC__ < 13) && !defined(__clang__)
    static_assert(
        dependent_false<T>,
#else   // defined(__GNUC__) && (__GNUC__ < 13) && !defined(__clang__)
    static_assert(
        false,
#endif  // !(defined(__GNUC__) && (__GNUC__ < 13) && !defined(__clang__))
        "Traits for this logger aren't visible. Are you missing an include of "
        "log_manager.h and/or log_buffer.h?");
    return {};
  }
};

// This class is used to explicitly ignore values in the conditional
// logging macros. This avoids compiler warnings like "value computed
// is not used" and "statement has no effect".
class Voidify {
 public:
  constexpr Voidify() = default;
  // This has to be an operator with a precedence lower than << but
  // higher than ?:
  template <typename U>
  void operator&(const U&) {}
};

}  // namespace autofill::internal

#endif  // COMPONENTS_AUTOFILL_CORE_COMMON_LOGGING_LOG_MACROS_H_
