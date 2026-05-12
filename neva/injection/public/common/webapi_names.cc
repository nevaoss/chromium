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

#include "neva/injection/public/common/webapi_names.h"

namespace injections {

namespace webapi {

const char kBrowserControl[] = "v8/browser_control";
const char kBrowserShell[] = "v8/browser_shell";
const char kBrowserShellIpc[] = "v8/browser_shell_ipc";
#if defined(USE_NEVA_CHROME_EXTENSIONS)
const char kChromeExtensions[] = "v8/chrome_extensions";
#endif  // defined(USE_NEVA_CHROME_EXTENSIONS)
const char kMemoryManager[] = "v8/memorymanager";
const char kNetworkErrorPage[] = "v8/networkerrorpage";
const char kSample[] = "v8/sample";
const char kWebOSGAV[] = "v8/webosgavplugin";
const char kWebOSServiceBridge[] = "v8/webosservicebridge";
const char kWebOSServiceBridgeObsolete[] = "v8/palmservicebridge";
const char kWebOSSystem[] = "v8/webossystem";
const char kWebOSSystemObsolete[]= "v8/palmsystem";

}  // namespace webapi

}  // namespace injections
