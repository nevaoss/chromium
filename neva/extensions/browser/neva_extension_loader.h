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

#ifndef NEVA_EXTENSIONS_BROWSER_NEVA_EXTENSION_LOADER_H_
#define NEVA_EXTENSIONS_BROWSER_NEVA_EXTENSION_LOADER_H_

#include "base/memory/raw_ptr.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "extensions/browser/extension_registrar.h"
#include "extensions/common/extension_id.h"

namespace base {
class FilePath;
}  // namespace base

namespace content {
class BrowserContext;
}  // namespace content

namespace neva {

class NevaExtensionLoader : public extensions::ExtensionRegistrar::Delegate {
 public:
  explicit NevaExtensionLoader(content::BrowserContext* browser_context);

  NevaExtensionLoader(const NevaExtensionLoader&) = delete;
  NevaExtensionLoader& operator=(const NevaExtensionLoader&) = delete;

  ~NevaExtensionLoader() override;

  // Loads an unpacked extension from a directory synchronously. Returns the
  // extension on success, or nullptr otherwise.
  const extensions::Extension* LoadExtension(
      const base::FilePath& extension_dir);

  // Starts reloading the extension. A keep-alive is maintained until the
  // reload succeeds/fails. If the extension is an app, it will be launched upon
  // reloading.
  // This may invalidate references to the old Extension object, so it takes the
  // ID by value.
  void ReloadExtension(extensions::ExtensionId extension_id);

 private:
  // If the extension loaded successfully, enables it. If it's an app, launches
  // it. If the load failed, updates ShellKeepAliveRequester.
  void FinishExtensionReload(
      const extensions::ExtensionId old_extension_id,
      scoped_refptr<const extensions::Extension> extension);

  // ExtensionRegistrar::Delegate:
  void PreAddExtension(const extensions::Extension* extension,
                       const extensions::Extension* old_extension) override;
  void PostActivateExtension(
      scoped_refptr<const extensions::Extension> extension) override;
  void PostDeactivateExtension(
      scoped_refptr<const extensions::Extension> extension) override;
  void LoadExtensionForReload(const extensions::ExtensionId& extension_id,
                              const base::FilePath& path,
                              extensions::ExtensionRegistrar::LoadErrorBehavior
                                  load_error_behavior) override;
  bool CanEnableExtension(const extensions::Extension* extension) override;
  bool CanDisableExtension(const extensions::Extension* extension) override;
  bool ShouldBlockExtension(const extensions::Extension* extension) override;

  raw_ptr<content::BrowserContext> browser_context_;  // Not owned.

  // Registers and unregisters extensions.
  extensions::ExtensionRegistrar extension_registrar_;

  // Indicates that we posted the (asynchronous) task to start reloading.
  // Used by ReloadExtension() to check whether ExtensionRegistrar calls
  // LoadExtensionForReload().
  bool did_schedule_reload_ = false;

  base::WeakPtrFactory<NevaExtensionLoader> weak_factory_{this};
};

}  // namespace neva

#endif  // NEVA_EXTENSIONS_BROWSER_NEVA_EXTENSION_LOADER_H_
