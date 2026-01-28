// Copyright 2022 LG Electronics, Inc.
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

#include "neva/app_runtime/browser/extensions/tab_helper_impl.h"

#include <optional>

#include "neva/app_runtime/app/app_runtime_page_contents.h"
#include "neva/app_runtime/app/app_runtime_page_view.h"
#include "neva/app_runtime/app/app_runtime_shell_environment.h"
#include "neva/extensions/common/api/windows.h"

namespace neva {

namespace windows = extensions::api::windows;

content::WebContents* TabHelperImpl::GetWebContentsFromId(uint64_t id) {
  neva_app_runtime::PageContents* page_contents =
      neva_app_runtime::ShellEnvironment::GetInstance()->GetContentsPtr(id);

  if (page_contents)
    return page_contents->GetWebContents();
  return nullptr;
}

uint64_t TabHelperImpl::GetIdFromWebContents(
    content::WebContents* web_contents) {
  return neva_app_runtime::ShellEnvironment::GetInstance()->GetID(web_contents);
}

uint64_t TabHelperImpl::GetTabIdFromWebContents(
    content::WebContents* web_contents) {
  auto* shell = neva_app_runtime::ShellEnvironment::GetInstance();
  auto* page_contents = shell->GetPageContentsFrom(web_contents);

  // ExtensionPopupWindow is not a tab.
  if (page_contents &&
      page_contents->GetType() ==
          neva_app_runtime::PageContents::Type::kExtensionPopupWindow) {
    return 0;
  }

  std::optional<uint64_t> tab_id = shell->GetTabID(web_contents);
  return tab_id.value_or(0);
}

views::View* TabHelperImpl::GetViewFromId(uint64_t view_id) {
  neva_app_runtime::PageView* page_view =
      neva_app_runtime::ShellEnvironment::GetInstance()->GetViewPtr(view_id);

  if (page_view)
    return page_view->GetView();
  return nullptr;
}

std::optional<windows::WindowType> TabHelperImpl::GetExtensionWindowType(
    uint64_t tab_id) {
  neva_app_runtime::PageContents::Type tab_type =
      neva_app_runtime::ShellEnvironment::GetInstance()->GetPageContentType(
          tab_id);
  if (tab_type == neva_app_runtime::PageContents::Type::kExtensionPopup) {
    return windows::WindowType::kPopup;
  } else if (tab_type == neva_app_runtime::PageContents::Type::kExtensionNormal) {
    return windows::WindowType::kNormal;
  }

  return std::nullopt;
}

bool TabHelperImpl::IsExtensionTab(uint64_t tab_id) {
  return false;
}

std::string TabHelperImpl::GetBrowserTabType(windows::CreateType type) {
  neva_app_runtime::PageContents::Type ret;
  switch (type) {
    case windows::CreateType::kPopup:
      ret = neva_app_runtime::PageContents::Type::kExtensionPopup;
      break;
    case windows::CreateType::kNormal:
    default:
      ret = neva_app_runtime::PageContents::Type::kExtensionNormal;
      break;
  }
  return neva_app_runtime::PageContents::ConvertTypeToString(ret);
}

std::string TabHelperImpl::GetBrowserNormalTabType() {
  return neva_app_runtime::PageContents::ConvertTypeToString(
      neva_app_runtime::PageContents::Type::kTab);
}

}// namespace neva
