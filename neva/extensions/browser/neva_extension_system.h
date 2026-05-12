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

#ifndef NEVA_EXTENSIONS_BROWSER_NEVA_EXTENSION_SYSTEM_H_
#define NEVA_EXTENSIONS_BROWSER_NEVA_EXTENSION_SYSTEM_H_

#include "base/one_shot_event.h"
#include "extensions/browser/extension_system.h"
#include "neva/extensions/browser/neva_extension_loader.h"

namespace base {
class FilePath;
}

namespace content {
class BrowserContext;
}

namespace value_store {
class ValueStoreFactory;
}

namespace neva {

class NevaExtensionSystem : public extensions::ExtensionSystem {
 public:
  using InstallUpdateCallback =
      extensions::ExtensionSystem::InstallUpdateCallback;
  explicit NevaExtensionSystem(content::BrowserContext* browser_context);

  NevaExtensionSystem(const NevaExtensionSystem&) = delete;
  NevaExtensionSystem& operator=(const NevaExtensionSystem&) = delete;

  ~NevaExtensionSystem() override;

  // Loads an unpacked extension from a directory. Returns the extension on
  // success, or nullptr otherwise.
  const extensions::Extension* LoadExtension(
      const base::FilePath& extension_dir);

  // Finish initialization for the shell extension system.
  void FinishInitialization();

  // Reloads the extension with id |extension_id|.
  void ReloadExtension(const extensions::ExtensionId& extension_id);

  // KeyedService implementation:
  void Shutdown() override;

  // extensions::ExtensionSystem implementation:
  void InitForRegularProfile(bool extensions_enabled) override;
  extensions::ExtensionService* extension_service() override;
  extensions::ManagementPolicy* management_policy() override;
  extensions::ServiceWorkerManager* service_worker_manager() override;
  extensions::UserScriptManager* user_script_manager() override;
  extensions::StateStore* state_store() override;
  extensions::StateStore* rules_store() override;
  extensions::StateStore* dynamic_user_scripts_store() override;
  scoped_refptr<value_store::ValueStoreFactory> store_factory() override;
  extensions::QuotaService* quota_service() override;
  extensions::AppSorting* app_sorting() override;
  const base::OneShotEvent& ready() const override;
  bool is_ready() const override;
  extensions::ContentVerifier* content_verifier() override;
  std::unique_ptr<extensions::ExtensionSet> GetDependentExtensions(
      const extensions::Extension* extension) override;
  void InstallUpdate(const std::string& extension_id,
                     const std::string& public_key,
                     const base::FilePath& temp_dir,
                     bool install_immediately,
                     InstallUpdateCallback install_update_callback) override;
  void PerformActionBasedOnOmahaAttributes(
      const std::string& extension_id,
      const base::Value::Dict& attributes) override;
  bool FinishDelayedInstallationIfReady(const std::string& extension_id,
                                        bool install_immediately) override;

 private:
  void OnExtensionRegisteredWithRequestContexts(
      scoped_refptr<extensions::Extension> extension);
  raw_ptr<content::BrowserContext> browser_context_;  // Not owned.

  std::unique_ptr<extensions::ServiceWorkerManager> service_worker_manager_;
  std::unique_ptr<extensions::QuotaService> quota_service_;
  std::unique_ptr<extensions::AppSorting> app_sorting_;
  std::unique_ptr<extensions::UserScriptManager> user_script_manager_;
  std::unique_ptr<extensions::ManagementPolicy> management_policy_;

  std::unique_ptr<NevaExtensionLoader> extension_loader_;

  scoped_refptr<value_store::ValueStoreFactory> store_factory_;

  // Signaled when the extension system has completed its startup tasks.
  base::OneShotEvent ready_;

  base::WeakPtrFactory<NevaExtensionSystem> weak_factory_{this};
};

}  // namespace neva

#endif  // NEVA_EXTENSIONS_BROWSER_NEVA_EXTENSION_SYSTEM_H_
