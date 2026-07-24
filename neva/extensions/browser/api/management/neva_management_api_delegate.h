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

#ifndef NEVA_EXTENSIONS_BROWSER_API_MANAGEMENT_NEVA_MANAGEMENT_API_DELEGATE_H_
#define NEVA_EXTENSIONS_BROWSER_API_MANAGEMENT_NEVA_MANAGEMENT_API_DELEGATE_H_

#include "extensions/browser/api/management/management_api_delegate.h"

namespace neva {

class NevaManagementAPIDelegate : public extensions::ManagementAPIDelegate {
 public:
  NevaManagementAPIDelegate();

  NevaManagementAPIDelegate(const NevaManagementAPIDelegate&) = delete;
  NevaManagementAPIDelegate& operator=(const NevaManagementAPIDelegate&) =
      delete;

  ~NevaManagementAPIDelegate() override;

  // ManagementAPIDelegate.
  bool LaunchAppFunctionDelegate(
      const extensions::Extension* extension,
      content::BrowserContext* context) const override;
  GURL GetFullLaunchURL(const extensions::Extension* extension) const override;
  extensions::LaunchType GetLaunchType(
      const extensions::ExtensionPrefs* prefs,
      const extensions::Extension* extension) const override;
  std::unique_ptr<extensions::InstallPromptDelegate> SetEnabledFunctionDelegate(
      content::WebContents* web_contents,
      content::BrowserContext* browser_context,
      const extensions::Extension* extension,
      base::OnceCallback<void(bool)> callback) const override;
  void EnableExtension(content::BrowserContext* context,
                       const std::string& extension_id) const override;
  void DisableExtension(
      content::BrowserContext* context,
      const extensions::Extension* source_extension,
      const std::string& extension_id,
      extensions::disable_reason::DisableReason disable_reason) const override;
  std::unique_ptr<extensions::UninstallDialogDelegate>
  UninstallFunctionDelegate(
      extensions::ManagementUninstallFunctionBase* function,
      const extensions::Extension* target_extension,
      bool show_programmatic_uninstall_ui) const override;
  bool UninstallExtension(content::BrowserContext* context,
                          const std::string& transient_extension_id,
                          extensions::UninstallReason reason,
                          std::u16string* error) const override;
  bool CreateAppShortcutFunctionDelegate(
      extensions::ManagementCreateAppShortcutFunction* function,
      const extensions::Extension* extension,
      std::string* error) const override;
  void SetLaunchType(content::BrowserContext* context,
                     const std::string& extension_id,
                     extensions::LaunchType launch_type) const override;
  std::unique_ptr<extensions::AppForLinkDelegate>
  GenerateAppForLinkFunctionDelegate(
      extensions::ManagementGenerateAppForLinkFunction* function,
      content::BrowserContext* context,
      const std::string& title,
      const GURL& launch_url) const override;
  bool CanContextInstallWebApps(
      content::BrowserContext* context) const override;
  void InstallOrLaunchReplacementWebApp(
      content::BrowserContext* context,
      const GURL& web_app_url,
      InstallOrLaunchWebAppCallback callback) const override;
  GURL GetIconURL(const extensions::Extension* extension,
                  int icon_size,
                  ExtensionIconSet::Match match,
                  bool grayscale) const override;
  GURL GetEffectiveUpdateURL(const extensions::Extension& extension,
                             content::BrowserContext* context) const override;
};

}  // namespace neva

#endif  // NEVA_EXTENSIONS_BROWSER_API_MANAGEMENT_NEVA_MANAGEMENT_API_DELEGATE_H_
