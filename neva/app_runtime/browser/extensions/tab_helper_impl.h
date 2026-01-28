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

#ifndef NEVA_APP_RUNTIME_BROWSER_EXTENSIONS_TAB_HELPER_IMPL_H_
#define NEVA_APP_RUNTIME_BROWSER_EXTENSIONS_TAB_HELPER_IMPL_H_

#include "neva/extensions/browser/tab_helper.h"

#include <stdint.h>
#include <optional>

namespace content {
class WebContents;
}

namespace views {
class View;
}

namespace extensions {
namespace api {
namespace windows {
enum class WindowType;
enum class CreateType;
}  // namespace windows
}  // namespace api
}  // namespace extensions

namespace neva {

class TabHelperImpl : public TabHelper {
 public:
  TabHelperImpl() = default;
  TabHelperImpl(const TabHelperImpl&) = delete;
  TabHelperImpl& operator=(const TabHelperImpl&) = delete;
  ~TabHelperImpl() override = default;

  content::WebContents* GetWebContentsFromId(uint64_t id) override;
  uint64_t GetIdFromWebContents(content::WebContents* web_contents) override;
  uint64_t GetTabIdFromWebContents(content::WebContents* web_contents) override;
  views::View* GetViewFromId(uint64_t view_id) override;
  bool IsExtensionTab(uint64_t tab_id) override;
  std::optional<extensions::api::windows::WindowType> GetExtensionWindowType(
      uint64_t tab_id) override;
  std::string GetBrowserTabType(extensions::api::windows::CreateType type) override;
  std::string GetBrowserNormalTabType() override;
};

}  // namespace neva

#endif  // NEVA_APP_RUNTIME_BROWSER_EXTENSIONS_TAB_HELPER_IMPL_H_
