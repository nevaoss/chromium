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

// Constants used for the Tabs API and the Windows API.

#ifndef NEVA_EXTENSIONS_BROWSER_API_TABS_TABS_CONSTANTS_H_
#define NEVA_EXTENSIONS_BROWSER_API_TABS_TABS_CONSTANTS_H_

namespace neva {
namespace tabs_constants {

extern const char kIsWindowClosingKey[];
extern const char kTabIdKey[];
extern const char kStatusKey[];
extern const char kStatusComplete[];
extern const char kStatusLoading[];
extern const char kWindowIdKey[];

// Error messages.
extern const char kCannotDetermineLanguageOfUnloadedTab[];
extern const char kCannotNavigateToChromeUntrusted[];
extern const char kCannotNavigateToDevtools[];
extern const char kFileUrlsNotAllowedInExtensionNavigations[];
extern const char kFrameNotFoundError[];
extern const char kJavaScriptUrlsNotAllowedInExtensionNavigations[];
extern const char kNoActiveWebContentsToCapture[];
extern const char kNoCrashBrowserError[];
extern const char kNoCurrentTabError[];
extern const char kNoSelectedTabError[];
extern const char kScreenshotsDisabled[];
extern const char kScreenshotsDisabledByDlp[];
extern const char kTabNotFoundError[];
extern const char kInvalidUrlError[];

}  // namespace tabs_constants
}  // namespace neva

#endif  // NEVA_EXTENSIONS_BROWSER_API_TABS_TABS_CONSTANTS_H_
