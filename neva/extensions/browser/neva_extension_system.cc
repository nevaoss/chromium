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

#include "neva/extensions/browser/neva_extension_system.h"

#include <memory>
#include <string>

#include "apps/launcher.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/functional/bind.h"
#include "components/value_store/value_store_factory_impl.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "extensions/browser/api/app_runtime/app_runtime_api.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/management_policy.h"
#include "extensions/browser/null_app_sorting.h"
#include "extensions/browser/quota_service.h"
#include "extensions/browser/service_worker_manager.h"
#include "extensions/browser/user_script_manager.h"
#include "extensions/common/constants.h"
#include "extensions/common/file_util.h"

using content::BrowserContext;

namespace neva {

NevaExtensionSystem::NevaExtensionSystem(BrowserContext* browser_context)
    : browser_context_(browser_context),
      store_factory_(
          new value_store::ValueStoreFactoryImpl(browser_context->GetPath())) {}

NevaExtensionSystem::~NevaExtensionSystem() = default;

const extensions::Extension* NevaExtensionSystem::LoadExtension(
    const base::FilePath& extension_dir) {
  return extension_loader_->LoadExtension(extension_dir);
}

void NevaExtensionSystem::FinishInitialization() {
  // Inform the rest of the extensions system to start.
  ready_.Signal();
}

void NevaExtensionSystem::ReloadExtension(
    const extensions::ExtensionId& extension_id) {
  extension_loader_->ReloadExtension(extension_id);
}

void NevaExtensionSystem::Shutdown() {
  extension_loader_.reset();
}

void NevaExtensionSystem::InitForRegularProfile(bool extensions_enabled) {
  if (!extensions_enabled) {
    return;
  }
  service_worker_manager_ =
      std::make_unique<extensions::ServiceWorkerManager>(browser_context_);
  quota_service_ = std::make_unique<extensions::QuotaService>();
  app_sorting_ = std::make_unique<extensions::NullAppSorting>();
  extension_loader_ = std::make_unique<NevaExtensionLoader>(browser_context_);
  user_script_manager_ =
      std::make_unique<extensions::UserScriptManager>(browser_context_);
  management_policy_ = std::make_unique<extensions::ManagementPolicy>();
}

extensions::ExtensionService* NevaExtensionSystem::extension_service() {
  return nullptr;
}

extensions::ManagementPolicy* NevaExtensionSystem::management_policy() {
  return management_policy_.get();
}

extensions::ServiceWorkerManager*
NevaExtensionSystem::service_worker_manager() {
  return service_worker_manager_.get();
}

extensions::UserScriptManager* NevaExtensionSystem::user_script_manager() {
  return user_script_manager_.get();
}

extensions::StateStore* NevaExtensionSystem::state_store() {
  return nullptr;
}

extensions::StateStore* NevaExtensionSystem::rules_store() {
  return nullptr;
}

extensions::StateStore* NevaExtensionSystem::dynamic_user_scripts_store() {
  return nullptr;
}

scoped_refptr<value_store::ValueStoreFactory>
NevaExtensionSystem::store_factory() {
  return store_factory_;
}

extensions::QuotaService* NevaExtensionSystem::quota_service() {
  return quota_service_.get();
}

extensions::AppSorting* NevaExtensionSystem::app_sorting() {
  return app_sorting_.get();
}

const base::OneShotEvent& NevaExtensionSystem::ready() const {
  return ready_;
}

bool NevaExtensionSystem::is_ready() const {
  return ready_.is_signaled();
}

extensions::ContentVerifier* NevaExtensionSystem::content_verifier() {
  return nullptr;
}

std::unique_ptr<extensions::ExtensionSet>
NevaExtensionSystem::GetDependentExtensions(
    const extensions::Extension* extension) {
  return std::make_unique<extensions::ExtensionSet>();
}

void NevaExtensionSystem::InstallUpdate(
    const std::string& extension_id,
    const std::string& public_key,
    const base::FilePath& temp_dir,
    bool install_immediately,
    InstallUpdateCallback install_update_callback) {
  NOTREACHED_IN_MIGRATION();
  base::DeletePathRecursively(temp_dir);
}

void NevaExtensionSystem::PerformActionBasedOnOmahaAttributes(
    const std::string& extension_id,
    const base::Value::Dict& attributes) {
  NOTREACHED_IN_MIGRATION();
}

bool NevaExtensionSystem::FinishDelayedInstallationIfReady(
    const std::string& extension_id,
    bool install_immediately) {
  NOTREACHED_IN_MIGRATION();
  return false;
}

void NevaExtensionSystem::OnExtensionRegisteredWithRequestContexts(
    scoped_refptr<extensions::Extension> extension) {
  extensions::ExtensionRegistry* registry =
      extensions::ExtensionRegistry::Get(browser_context_);
  registry->AddReady(extension);
  registry->TriggerOnReady(extension.get());
}

}  // namespace neva
