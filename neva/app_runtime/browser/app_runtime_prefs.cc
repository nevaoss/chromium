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

#include "neva/app_runtime/browser/app_runtime_prefs.h"

#include "base/files/file_path.h"
#include "base/memory/ref_counted.h"
#include "build/build_config.h"
#include "build/chromeos_buildflags.h"
#include "components/content_settings/core/browser/host_content_settings_map.h"
#include "components/language/core/browser/language_prefs.h"
#include "components/pref_registry/pref_registry_syncable.h"
#include "components/prefs/pref_filter.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/pref_service_factory.h"
#include "components/sessions/core/session_id_generator.h"
#include "components/user_prefs/user_prefs.h"
#include "content/public/browser/browser_context.h"

#if defined(USE_NEVA_CHROME_EXTENSIONS)
#include "extensions/browser/api/audio/audio_api.h"
#include "extensions/browser/extension_prefs.h"
#include "extensions/browser/permissions_manager.h"
#endif  // defined(USE_NEVA_CHROME_EXTENSIONS)

using base::FilePath;
using user_prefs::PrefRegistrySyncable;

namespace neva_app_runtime {
namespace {

void RegisterLocalStatePrefs(PrefRegistrySimple* registry) {
  sessions::SessionIdGenerator::RegisterPrefs(registry);
}

}  // namespace

namespace prefs {

scoped_refptr<JsonPrefStore> CreateAndLoadPrefStore(const FilePath& filepath) {
  scoped_refptr<JsonPrefStore> pref_store =
      base::MakeRefCounted<JsonPrefStore>(filepath);
  // NOTE(neva): Need to allow blocking of scope to use
  // synchronous JsonPrefStore::ReadPrefs().
  base::ScopedAllowBlocking allow_blocking;
  pref_store->ReadPrefs();  // Synchronous.
  return pref_store;
}

std::unique_ptr<PrefService> CreateLocalState(const FilePath& data_dir) {
  FilePath filepath = data_dir.AppendASCII("local_state.json");
  scoped_refptr<JsonPrefStore> pref_store = CreateAndLoadPrefStore(filepath);

  // Local state is considered "user prefs" from the factory's perspective.
  PrefServiceFactory factory;
  factory.set_user_prefs(pref_store);

  // Local state preferences are not syncable.
  PrefRegistrySimple* registry = new PrefRegistrySimple;
  RegisterLocalStatePrefs(registry);

  // NOTE(neva): Need to allow blocking of scope to use
  // synchronous JsonPrefStore::ReadPrefs().
  base::ScopedAllowBlocking allow_blocking;
  return factory.Create(registry);
}

std::unique_ptr<PrefService> CreateUserPrefService(
    content::BrowserContext* browser_context) {
  FilePath filepath = browser_context->GetPath().AppendASCII("user_prefs.json");
  scoped_refptr<JsonPrefStore> pref_store = CreateAndLoadPrefStore(filepath);

  PrefServiceFactory factory;
  factory.set_user_prefs(pref_store);

  // TODO(jamescook): If we want to support prefs that are set by extensions
  // via ChromeSettings properties (e.g. chrome.accessibilityFeatures or
  // chrome.proxy) then this should create an ExtensionPrefStore and attach it
  // with PrefServiceFactory::set_extension_prefs().
  // See https://developer.chrome.com/extensions/types#ChromeSetting

  // Prefs should be registered before the PrefService is created.
  PrefRegistrySyncable* pref_registry = new PrefRegistrySyncable;
#if defined(USE_NEVA_CHROME_EXTENSIONS)
  extensions::ExtensionPrefs::RegisterProfilePrefs(pref_registry);
  extensions::AudioAPI::RegisterUserPrefs(pref_registry);
  extensions::PermissionsManager::RegisterProfilePrefs(pref_registry);
#endif  // defined(USE_NEVA_CHROME_EXTENSIONS)
  language::LanguagePrefs::RegisterProfilePrefs(pref_registry);
  HostContentSettingsMap::RegisterProfilePrefs(pref_registry);

  // NOTE(neva): Need to allow blocking of scope to use
  // synchronous JsonPrefStore::ReadPrefs().
  std::unique_ptr<PrefService> pref_service;
  {
    base::ScopedAllowBlocking allow_blocking;
    pref_service = factory.Create(pref_registry);
  }
  user_prefs::UserPrefs::Set(browser_context, pref_service.get());
  return pref_service;
}

}  // namespace prefs

}  // namespace neva_app_runtime
