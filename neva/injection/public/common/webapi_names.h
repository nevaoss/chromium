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

#ifndef NEVA_INJECTION_PUBLIC_COMMON_WEBAPI_NAMES_H_
#define NEVA_INJECTION_PUBLIC_COMMON_WEBAPI_NAMES_H_

#include "base/component_export.h"

namespace injections {

namespace webapi {

COMPONENT_EXPORT(INJECTION) extern const char kBrowserControl[];
COMPONENT_EXPORT(INJECTION) extern const char kBrowserShell[];
COMPONENT_EXPORT(INJECTION) extern const char kBrowserShellIpc[];
#if defined(USE_NEVA_CHROME_EXTENSIONS)
COMPONENT_EXPORT(INJECTION) extern const char kChromeExtensions[];
#endif  // defined(USE_NEVA_CHROME_EXTENSIONS)
COMPONENT_EXPORT(INJECTION) extern const char kMemoryManager[];
COMPONENT_EXPORT(INJECTION) extern const char kNetworkErrorPage[];
COMPONENT_EXPORT(INJECTION) extern const char kSample[];
COMPONENT_EXPORT(INJECTION) extern const char kWebOSGAV[];
COMPONENT_EXPORT(INJECTION) extern const char kWebOSServiceBridge[];
COMPONENT_EXPORT(INJECTION) extern const char kWebOSServiceBridgeObsolete[];
COMPONENT_EXPORT(INJECTION) extern const char kWebOSSystem[];
COMPONENT_EXPORT(INJECTION) extern const char kWebOSSystemObsolete[];

}  // namespace webapi

}  // namespace injections

#endif  // NEVA_INJECTION_PUBLIC_COMMON_WEBAPI_NAMES_H_
