// Copyright 2024 LG Electronics, Inc.
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

// Constants used for the WebNavigation API.

#ifndef NEVA_EXTENSIONS_BROWSER_API_WEB_NAVIGATION_WEB_NAVIGATION_API_CONSTANTS_H_
#define NEVA_EXTENSIONS_BROWSER_API_WEB_NAVIGATION_WEB_NAVIGATION_API_CONSTANTS_H_

namespace neva {

namespace web_navigation_api_constants {

// Keys.
extern const char kDocumentIdKey[];
extern const char kDocumentLifecycleKey[];
extern const char kFrameIdKey[];
extern const char kFrameTypeKey[];
extern const char kParentDocumentIdKey[];
extern const char kParentFrameIdKey[];
extern const char kProcessIdKey[];
extern const char kTabIdKey[];
extern const char kTimeStampKey[];
extern const char kTransitionTypeKey[];
extern const char kTransitionQualifiersKey[];
extern const char kUrlKey[];

}  // namespace web_navigation_api_constants

}  // namespace neva

#endif  // NEVA_EXTENSIONS_BROWSER_API_WEB_NAVIGATION_WEB_NAVIGATION_API_CONSTANTS_H_
