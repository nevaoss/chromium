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

#ifndef NEVA_APP_RUNTIME_RENDERER_APP_RUNTIME_LOCALIZED_ERROR_H_
#define NEVA_APP_RUNTIME_RENDERER_APP_RUNTIME_LOCALIZED_ERROR_H_

#include <string>

#include "base/values.h"

namespace neva_app_runtime {

class AppRuntimeLocalizedError {
 public:
  AppRuntimeLocalizedError() = delete;
  AppRuntimeLocalizedError(const AppRuntimeLocalizedError&) = delete;
  AppRuntimeLocalizedError& operator=(const AppRuntimeLocalizedError&) = delete;
  // Fills |strings| with values to be used to build an error page used
  // on HTTP errors, like 404 or connection reset.
  static void GetErrorStrings(int error_code, base::Value::Dict& error_strings);
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_RENDERER_APP_RUNTIME_LOCALIZED_ERROR_H_
