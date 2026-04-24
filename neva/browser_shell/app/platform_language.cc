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

#include "neva/browser_shell/app/platform_language.h"

#include <string>

#include "base/functional/bind.h"
#include "base/logging.h"
#include "neva/app_runtime/app/app_runtime_page_contents.h"
#include "neva/app_runtime/app/app_runtime_shell_environment.h"
#include "neva/pal_service/pal_platform_factory.h"
#include "neva/pal_service/public/language_tracker_delegate.h"

#if defined(USE_NEVA_CHROME_EXTENSIONS)
#include "neva/extensions/browser/neva_extensions_service_factory.h"
#include "neva/extensions/browser/neva_extensions_service_impl.h"
#include "neva/extensions/browser/web_contents_map.h"
#endif  // defined(USE_NEVA_CHROME_EXTENSIONS)

namespace browser_shell {

namespace {

void UpdatePreferredLanguage(const std::string& language_string,
                             neva_app_runtime::PageContents* ptr) {
  ptr->UpdatePreferredLanguage(language_string);
}

#if defined(USE_NEVA_CHROME_EXTENSIONS)
void UpdatePreferredLanguageForExtension(const std::string& language,
                                         content::WebContents* web_contents) {
  neva::NevaExtensionsServiceImpl* neva_extensions_service_impl =
      neva::NevaExtensionsServiceFactory::GetService(
          web_contents->GetBrowserContext());
  neva_extensions_service_impl->SetPreferredLanguage(language);
  neva_extensions_service_impl->UpdatePreferredLanguage(web_contents);
}
#endif  // defined(USE_NEVA_CHROME_EXTENSIONS)

}  // namespace

PlatformLanguage::PlatformLanguage()
    : delegate_(pal::PlatformFactory::Get()->CreateLanguageTrackerDelegate(
          base::BindRepeating(&PlatformLanguage::OnLanguageChanged,
                              base::Unretained(this)))) {
  if (delegate_ &&
      delegate_->GetStatus() !=
          pal::LanguageTrackerDelegate::Status::kSuccess) {
    LOG(ERROR) << __func__ << "(): error during delegate creation";
  }
}

PlatformLanguage::~PlatformLanguage() = default;

void PlatformLanguage::OnLanguageChanged(const std::string& language_string) {
  if (is_closing_)
    return;

  neva_app_runtime::ShellEnvironment::GetInstance()->ForEachPageContents(
      base::BindRepeating(
          &UpdatePreferredLanguage, std::cref(language_string)));

#if defined(USE_NEVA_CHROME_EXTENSIONS)
  neva::WebContentsMap::GetInstance()->ForEachExtensionWebContents(
      base::BindRepeating(&UpdatePreferredLanguageForExtension,
                          std::cref(language_string)));
#endif  // defined(USE_NEVA_CHROME_EXTENSIONS)
}

void PlatformLanguage::OnMainWindowClosing() {
  is_closing_ = true;
}

}  // namespace browser_shell
