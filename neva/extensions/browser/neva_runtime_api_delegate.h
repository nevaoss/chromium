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

#ifndef NEVA_EXTENSIONS_BROWSER_NEVA_RUNTIME_API_DELEGATE_H_
#define NEVA_EXTENSIONS_BROWSER_NEVA_RUNTIME_API_DELEGATE_H_

#include "base/memory/raw_ptr.h"
#include "extensions/browser/api/runtime/runtime_api_delegate.h"

namespace content {
class BrowserContext;
}  // namespace content

namespace neva {

class NevaRuntimeAPIDelegate : public extensions::RuntimeAPIDelegate {
 public:
  explicit NevaRuntimeAPIDelegate(content::BrowserContext* browser_context);

  NevaRuntimeAPIDelegate(const NevaRuntimeAPIDelegate&) = delete;
  NevaRuntimeAPIDelegate& operator=(const NevaRuntimeAPIDelegate&) = delete;

  ~NevaRuntimeAPIDelegate() override;

  // RuntimeAPIDelegate implementation.
  void AddUpdateObserver(extensions::UpdateObserver* observer) override;
  void RemoveUpdateObserver(extensions::UpdateObserver* observer) override;
  void ReloadExtension(const std::string& extension_id) override;
  bool CheckForUpdates(const std::string& extension_id,
                       UpdateCheckCallback callback) override;
  void OpenURL(const GURL& uninstall_url) override;
  bool GetPlatformInfo(extensions::api::runtime::PlatformInfo* info) override;
  bool RestartDevice(std::string* error_message) override;

 private:
  raw_ptr<content::BrowserContext> browser_context_;
};

}  // namespace neva

#endif  // NEVA_EXTENSIONS_BROWSER_NEVA_RUNTIME_API_DELEGATE_H_
