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

#include "neva/extensions/browser/neva_runtime_api_delegate.h"

#include "extensions/common/api/runtime.h"
#include "neva/extensions/browser/neva_extension_system.h"
#include "neva/logging.h"

using extensions::api::runtime::PlatformInfo;

namespace neva {

NevaRuntimeAPIDelegate::NevaRuntimeAPIDelegate(
    content::BrowserContext* browser_context)
    : browser_context_(browser_context) {
  NEVA_DCHECK(browser_context_);
}

NevaRuntimeAPIDelegate::~NevaRuntimeAPIDelegate() = default;

void NevaRuntimeAPIDelegate::AddUpdateObserver(
    extensions::UpdateObserver* observer) {}

void NevaRuntimeAPIDelegate::RemoveUpdateObserver(
    extensions::UpdateObserver* observer) {}

void NevaRuntimeAPIDelegate::ReloadExtension(const std::string& extension_id) {
  static_cast<NevaExtensionSystem*>(
      extensions::ExtensionSystem::Get(browser_context_))
      ->ReloadExtension(extension_id);
}

bool NevaRuntimeAPIDelegate::CheckForUpdates(const std::string& extension_id,
                                             UpdateCheckCallback callback) {
  return false;
}

void NevaRuntimeAPIDelegate::OpenURL(const GURL& uninstall_url) {}

bool NevaRuntimeAPIDelegate::GetPlatformInfo(PlatformInfo* info) {
  info->os = extensions::api::runtime::PlatformOs::kLinux;
  return true;
}

bool NevaRuntimeAPIDelegate::RestartDevice(std::string* error_message) {
  // We allow chrome.runtime.restart() to request a device restart on ChromeOS.
  *error_message = "Restart is only supported on ChromeOS.";
  return false;
}

}  // namespace neva
