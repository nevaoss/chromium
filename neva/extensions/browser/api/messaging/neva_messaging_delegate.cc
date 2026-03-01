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

#include "neva/extensions/browser/api/messaging/neva_messaging_delegate.h"

#include <optional>

#include "content/public/browser/web_contents.h"
#include "neva/extensions/browser/neva_extensions_service_factory.h"
#include "neva/extensions/browser/neva_extensions_service_impl.h"
#include "neva/extensions/browser/tab_helper.h"

namespace neva {

NevaMessagingDelegate::NevaMessagingDelegate() = default;
NevaMessagingDelegate::~NevaMessagingDelegate() = default;

std::optional<base::DictValue> NevaMessagingDelegate::MaybeGetTabInfo(
    content::WebContents* web_contents) {
  // Add info about the opener's tab (if it was a tab).
  if (!web_contents) {
    return std::nullopt;
  }

  TabHelper* tab_helper = NevaExtensionsServiceFactory::GetService(
                              web_contents->GetBrowserContext())
                              ->GetTabHelper();

  if (tab_helper) {
    // Give only id and url from the Tab properties(tabs.json), at the moment.
    base::DictValue tab_info;
    tab_info.Set(
        "id", static_cast<int>(tab_helper->GetIdFromWebContents(web_contents)));
    tab_info.Set("url", web_contents->GetLastCommittedURL().spec());
    return tab_info;
  }

  return std::nullopt;
}

content::WebContents* NevaMessagingDelegate::GetWebContentsByTabId(
    content::BrowserContext* browser_context,
    int tab_id) {
  TabHelper* tab_helper =
      NevaExtensionsServiceFactory::GetService(browser_context)->GetTabHelper();

  if (tab_helper) {
    return tab_helper->GetWebContentsFromId(tab_id);
  }

  return nullptr;
}

}  // namespace neva
