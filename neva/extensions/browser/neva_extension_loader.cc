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

#include "neva/extensions/browser/neva_extension_loader.h"

#include "base/auto_reset.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/functional/bind.h"
#include "base/logging.h"
#include "base/task/sequenced_task_runner.h"
#include "extensions/browser/extension_file_task_runner.h"
#include "extensions/browser/extension_prefs.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/common/file_util.h"
#include "extensions/common/mojom/manifest.mojom-shared.h"
#include "extensions/common/permissions/permissions_data.h"
#include "extensions/common/url_pattern_set.h"

namespace neva {

using LoadErrorBehavior = extensions::ExtensionRegistrar::LoadErrorBehavior;

namespace {

scoped_refptr<const extensions::Extension> LoadUnpacked(
    const base::FilePath& extension_dir) {
  // app_shell only supports unpacked extensions.
  // NOTE: If you add packed extension support consider removing the flag
  // FOLLOW_SYMLINKS_ANYWHERE below. Packed extensions should not have symlinks.
  if (!base::DirectoryExists(extension_dir)) {
    LOG(ERROR) << "Extension directory not found: "
               << extension_dir.AsUTF8Unsafe();
    return nullptr;
  }

  int load_flags = extensions::Extension::FOLLOW_SYMLINKS_ANYWHERE;
  std::string load_error;
  scoped_refptr<extensions::Extension> extension =
      extensions::file_util::LoadExtension(
          extension_dir, extensions::mojom::ManifestLocation::kCommandLine,
          load_flags, &load_error);
  if (!extension.get()) {
    LOG(ERROR) << "Loading extension at " << extension_dir.value()
               << " failed with: " << load_error;
    return nullptr;
  }

  // Log warnings.
  if (extension->install_warnings().size()) {
    LOG(WARNING) << "Warnings loading extension at " << extension_dir.value()
                 << ":";
    for (const auto& warning : extension->install_warnings())
      LOG(WARNING) << warning.message;
  }

  return extension;
}

}  // namespace

NevaExtensionLoader::NevaExtensionLoader(
    content::BrowserContext* browser_context)
    : browser_context_(browser_context),
      extension_registrar_(browser_context, this) {}

NevaExtensionLoader::~NevaExtensionLoader() = default;

const extensions::Extension* NevaExtensionLoader::LoadExtension(
    const base::FilePath& extension_dir) {
  scoped_refptr<const extensions::Extension> extension =
      LoadUnpacked(extension_dir);
  if (extension) {
    // Provide (over)permission for all loaded extensions to pass
    // PermissionsData::CanAccessPage(). In chrome browser, such action is done
    // via ActiveTabPermissionGranter. But we don't have such mechanism yet.
    // TODO(neva): Remove this once we make alternative way.
    std::unique_ptr<const extensions::PermissionSet> withheld =
        extension->permissions_data()->withheld_permissions().Clone();
    std::unique_ptr<const extensions::PermissionSet> active =
        extension->permissions_data()->active_permissions().Clone();

    extensions::URLPatternSet allowed_url_patterns;
    allowed_url_patterns.AddPattern(
        URLPattern(URLPattern::SchemeMasks::SCHEME_HTTP, "http://*/*"));
    allowed_url_patterns.AddPattern(
        URLPattern(URLPattern::SchemeMasks::SCHEME_HTTPS, "https://*/*"));
    extensions::PermissionSet new_permission_set(
        {}, {}, std::move(allowed_url_patterns), {});

    std::unique_ptr<const extensions::PermissionSet> new_active =
        extensions::PermissionSet::CreateUnion(*active, new_permission_set);

    extension->permissions_data()->SetPermissions(std::move(new_active),
                                                  std::move(withheld));

    extension_registrar_.AddExtension(extension);
  }

  return extension.get();
}

void NevaExtensionLoader::ReloadExtension(
    extensions::ExtensionId extension_id) {
  const extensions::Extension* extension =
      extensions::ExtensionRegistry::Get(browser_context_)
          ->GetInstalledExtension(extension_id);
  // We shouldn't be trying to reload extensions that haven't been added.
  DCHECK(extension);

  // This should always start false since it's only set here, or in
  // LoadExtensionForReload() as a result of the call below.
  DCHECK_EQ(false, did_schedule_reload_);
  base::AutoReset<bool> reset_did_schedule_reload(&did_schedule_reload_, false);

  extension_registrar_.ReloadExtension(extension_id, LoadErrorBehavior::kQuiet);
  // if (did_schedule_reload_)
  //   return;
}

void NevaExtensionLoader::FinishExtensionReload(
    const extensions::ExtensionId old_extension_id,
    scoped_refptr<const extensions::Extension> extension) {
  if (extension) {
    extension_registrar_.AddExtension(extension);
  }
}

void NevaExtensionLoader::PreAddExtension(
    const extensions::Extension* extension,
    const extensions::Extension* old_extension) {
  if (old_extension)
    return;

  // The extension might be disabled if a previous reload attempt failed. In
  // that case, we want to remove that disable reason.
  extensions::ExtensionPrefs* extension_prefs =
      extensions::ExtensionPrefs::Get(browser_context_);
  if (extension_prefs->IsExtensionDisabled(extension->id()) &&
      extension_prefs->HasDisableReason(
          extension->id(), extensions::disable_reason::DISABLE_RELOAD)) {
    extension_prefs->RemoveDisableReason(
        extension->id(), extensions::disable_reason::DISABLE_RELOAD);
    // Only re-enable the extension if there are no other disable reasons.
    if (extension_prefs->GetDisableReasons(extension->id()) ==
        extensions::disable_reason::DISABLE_NONE) {
      extension_prefs->SetExtensionEnabled(extension->id());
    }
  }
}

void NevaExtensionLoader::PostActivateExtension(
    scoped_refptr<const extensions::Extension> extension) {}

void NevaExtensionLoader::PostDeactivateExtension(
    scoped_refptr<const extensions::Extension> extension) {}

void NevaExtensionLoader::LoadExtensionForReload(
    const extensions::ExtensionId& extension_id,
    const base::FilePath& path,
    LoadErrorBehavior load_error_behavior) {
  CHECK(!path.empty());

  extensions::GetExtensionFileTaskRunner()->PostTaskAndReplyWithResult(
      FROM_HERE, base::BindOnce(&LoadUnpacked, path),
      base::BindOnce(&NevaExtensionLoader::FinishExtensionReload,
                     weak_factory_.GetWeakPtr(), extension_id));
  did_schedule_reload_ = true;
}

bool NevaExtensionLoader::CanEnableExtension(
    const extensions::Extension* extension) {
  return true;
}

bool NevaExtensionLoader::CanDisableExtension(
    const extensions::Extension* extension) {
  // Extensions cannot be disabled by the user.
  return false;
}

bool NevaExtensionLoader::ShouldBlockExtension(
    const extensions::Extension* extension) {
  return false;
}

}  // namespace neva
