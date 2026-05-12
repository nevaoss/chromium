// Copyright 2023 LG Electronics, Inc.
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

#ifndef NEVA_EXTENSIONS_BROWSER_EXTENSION_TAB_UTIL_H_
#define NEVA_EXTENSIONS_BROWSER_EXTENSION_TAB_UTIL_H_

#include "base/types/expected.h"
#include "base/values.h"
#include "neva/extensions/common/api/tabs.h"
#include "neva/extensions/common/api/windows.h"

class GURL;

namespace content {
class BrowserContext;
class WebContents;
}  // namespace content

namespace extensions {
class Extension;
}

namespace neva {

class ExtensionTabUtil {
 public:
  static base::Value::Dict CreateWindowValueForExtension(
      int window_id,
      bool focused,
      extensions::api::windows::WindowType type =
          extensions::api::windows::WindowType::kPopup,
      bool always_on_top = false,
      bool incognito = false);
  static int GetTabId(content::WebContents* web_contents);
  static extensions::api::tabs::Tab CreateTabObject(
      content::WebContents* contents);

  // Takes |url_string| and returns a GURL which is either valid and absolute
  // or invalid. If |url_string| is not directly interpretable as a valid (it is
  // likely a relative URL) an attempt is made to resolve it. When |extension|
  // is non-null, the URL is resolved relative to its extension base
  // (chrome-extension://<id>/). Using the source frame url would be more
  // correct, but because the api shipped with urls resolved relative to their
  // extension base, we decided it wasn't worth breaking existing extensions to
  // fix.
  static GURL ResolvePossiblyRelativeURL(
      const std::string& url_string,
      const extensions::Extension* extension);

  // Returns true if navigating to |url| could kill a page or the browser
  // itself, whether by simulating a crash, browser quit, thread hang, or
  // equivalent. Extensions should be prevented from navigating to such URLs.
  //
  // The caller should ensure that |url| has already been "fixed up" by calling
  // url_formatter::FixupURL.
  static bool IsKillURL(const GURL& url);

  // Resolves the URL and ensures the extension is allowed to navigate to it.
  // Returns the url if successful, otherwise returns an error string.
  static base::expected<GURL, std::string> PrepareURLForNavigation(
      const std::string& url_string,
      const extensions::Extension* extension,
      content::BrowserContext* browser_context);

  // Determines the loading status of the given |contents|. This needs to access
  // some non-const member functions of |contents|, but actually leaves it
  // unmodified.
  static extensions::api::tabs::TabStatus GetLoadingStatus(
      content::WebContents* contents);

  static void DispatchTabsOnCreated(content::BrowserContext* context,
                                    uint64_t tab_id);
  static void DispatchTabsOnActivated(content::BrowserContext* context,
                                      uint64_t tab_id);
  static void DispatchTabsOnUpdated(content::BrowserContext* context,
                                    uint64_t tab_id,
                                    const std::string& change_info);
  static void DispatchTabsOnRemoved(content::BrowserContext* context,
                                    uint64_t tab_id);
  static void DispatchWindowsOnCreated(content::BrowserContext* context,
                                       uint64_t tab_id);
  static void DispatchWindowsOnFocusChanged(content::BrowserContext* context,
                                            uint64_t tab_id);
  static void DispatchWindowsOnRemoved(content::BrowserContext* context,
                                       uint64_t tab_id);
};

}  // namespace neva

#endif  // NEVA_EXTENSIONS_BROWSER_EXTENSION_TAB_UTIL_H_
