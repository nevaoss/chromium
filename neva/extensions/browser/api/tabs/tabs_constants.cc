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

#include "neva/extensions/browser/api/tabs/tabs_constants.h"

namespace neva {
namespace tabs_constants {

const char kIsWindowClosingKey[] = "isWindowClosing";
const char kTabIdKey[] = "tabId";
const char kStatusKey[] = "status";
const char kStatusComplete[] = "complete";
const char kStatusLoading[] = "loading";
const char kWindowIdKey[] = "windowId";
const char kCannotDetermineLanguageOfUnloadedTab[] =
    "Cannot determine language: tab not loaded";
const char kCannotNavigateToChromeUntrusted[] =
    "Cannot navigate to a chrome-untrusted:// page.";
const char kCannotNavigateToDevtools[] =
    "Cannot navigate to a devtools:// page without either the devtools or "
    "debugger permission.";
const char kFileUrlsNotAllowedInExtensionNavigations[] =
    "Cannot navigate to a file URL without local file access.";
const char kFrameNotFoundError[] = "No frame with id * in tab *.";
const char kJavaScriptUrlsNotAllowedInExtensionNavigations[] =
    "JavaScript URLs are not allowed in API based extension navigations. Use "
    "chrome.scripting.executeScript instead.";
const char kNoActiveWebContentsToCapture[] =
    "No active web contents to capture";
const char kNoCrashBrowserError[] = "I'm sorry. I'm afraid I can't do that.";
const char kNoCurrentTabError[] = "No current tab";
const char kNoSelectedTabError[] = "No selected tab";
const char kScreenshotsDisabled[] = "Taking screenshots has been disabled";
const char kScreenshotsDisabledByDlp[] =
    "Administrator policy disables screen capture when confidential content is "
    "visible";
const char kTabNotFoundError[] = "No tab with id: *.";
const char kInvalidUrlError[] = "Invalid url: \"*\".";

}  // namespace tabs_constants
}  // namespace neva
