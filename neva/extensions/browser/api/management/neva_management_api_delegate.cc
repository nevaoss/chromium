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

#include "neva/extensions/browser/api/management/neva_management_api_delegate.h"

namespace neva {

NevaManagementAPIDelegate::NevaManagementAPIDelegate() = default;

NevaManagementAPIDelegate::~NevaManagementAPIDelegate() = default;

bool NevaManagementAPIDelegate::LaunchAppFunctionDelegate(
    const extensions::Extension* extension,
    content::BrowserContext* context) const {
  return false;
}

GURL NevaManagementAPIDelegate::GetFullLaunchURL(
    const extensions::Extension* extension) const {
  return GURL{};
}

extensions::LaunchType NevaManagementAPIDelegate::GetLaunchType(
    const extensions::ExtensionPrefs* prefs,
    const extensions::Extension* extension) const {
  return extensions::LAUNCH_TYPE_REGULAR;
}

std::unique_ptr<extensions::InstallPromptDelegate>
NevaManagementAPIDelegate::SetEnabledFunctionDelegate(
    content::WebContents* web_contents,
    content::BrowserContext* browser_context,
    const extensions::Extension* extension,
    base::OnceCallback<void(bool)> callback) const {
  return nullptr;
}

void NevaManagementAPIDelegate::EnableExtension(
    content::BrowserContext* context,
    const std::string& extension_id) const {}

void NevaManagementAPIDelegate::DisableExtension(
    content::BrowserContext* context,
    const extensions::Extension* source_extension,
    const std::string& extension_id,
    extensions::disable_reason::DisableReason disable_reason) const {}

std::unique_ptr<extensions::UninstallDialogDelegate>
NevaManagementAPIDelegate::UninstallFunctionDelegate(
    extensions::ManagementUninstallFunctionBase* function,
    const extensions::Extension* target_extension,
    bool show_programmatic_uninstall_ui) const {
  return nullptr;
}

bool NevaManagementAPIDelegate::UninstallExtension(
    content::BrowserContext* context,
    const std::string& transient_extension_id,
    extensions::UninstallReason reason,
    std::u16string* error) const {
  return false;
}

bool NevaManagementAPIDelegate::CreateAppShortcutFunctionDelegate(
    extensions::ManagementCreateAppShortcutFunction* function,
    const extensions::Extension* extension,
    std::string* error) const {
  return false;
}

void NevaManagementAPIDelegate::SetLaunchType(
    content::BrowserContext* context,
    const std::string& extension_id,
    extensions::LaunchType launch_type) const {}

std::unique_ptr<extensions::AppForLinkDelegate>
NevaManagementAPIDelegate::GenerateAppForLinkFunctionDelegate(
    extensions::ManagementGenerateAppForLinkFunction* function,
    content::BrowserContext* context,
    const std::string& title,
    const GURL& launch_url) const {
  return nullptr;
}

bool NevaManagementAPIDelegate::CanContextInstallWebApps(
    content::BrowserContext* context) const {
  return false;
}

void NevaManagementAPIDelegate::InstallOrLaunchReplacementWebApp(
    content::BrowserContext* context,
    const GURL& web_app_url,
    InstallOrLaunchWebAppCallback callback) const {}

GURL NevaManagementAPIDelegate::GetIconURL(
    const extensions::Extension* extension,
    int icon_size,
    ExtensionIconSet::Match match,
    bool grayscale) const {
  return GURL{};
}

GURL NevaManagementAPIDelegate::GetEffectiveUpdateURL(
    const extensions::Extension& extension,
    content::BrowserContext* context) const {
  return GURL{};
}

}  // namespace neva
