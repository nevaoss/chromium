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

#include "neva/extensions/common/neva_extensions_client.h"

#include <memory>
#include <string>

#include "base/check.h"
#include "base/lazy_instance.h"
#include "base/notreached.h"
#include "components/version_info/version_info.h"
#include "content/public/common/user_agent.h"
#include "extensions/common/core_extensions_api_provider.h"
#include "extensions/common/extension_urls.h"
#include "extensions/common/features/simple_feature.h"
#include "extensions/common/permissions/permission_message_provider.h"
#include "extensions/common/url_pattern_set.h"
#include "neva/extensions/common/neva_extensions_api_provider.h"

namespace neva {

namespace {

// TODO(jamescook): Refactor ChromePermissionsMessageProvider so we can share
// code. For now, this implementation does nothing.
class NevaPermissionMessageProvider
    : public extensions::PermissionMessageProvider {
 public:
  NevaPermissionMessageProvider() = default;
  NevaPermissionMessageProvider(const NevaPermissionMessageProvider&) = delete;
  NevaPermissionMessageProvider& operator=(
      const NevaPermissionMessageProvider&) = delete;
  ~NevaPermissionMessageProvider() override = default;

  // PermissionMessageProvider implementation.
  extensions::PermissionMessages GetPermissionMessages(
      const extensions::PermissionIDSet& permissions) const override {
    return extensions::PermissionMessages();
  }

  bool IsPrivilegeIncrease(
      const extensions::PermissionSet& granted_permissions,
      const extensions::PermissionSet& requested_permissions,
      extensions::Manifest::Type extension_type) const override {
    // Ensure we implement this before shipping.
    CHECK(false);
    return false;
  }

  extensions::PermissionIDSet GetAllPermissionIDs(
      const extensions::PermissionSet& permissions,
      extensions::Manifest::Type extension_type) const override {
    return extensions::PermissionIDSet();
  }
};

base::LazyInstance<NevaPermissionMessageProvider>::DestructorAtExit
    g_permission_message_provider = LAZY_INSTANCE_INITIALIZER;

}  // namespace

NevaExtensionsClient::NevaExtensionsClient() {
  AddAPIProvider(std::make_unique<extensions::CoreExtensionsAPIProvider>());
  AddAPIProvider(std::make_unique<NevaExtensionsAPIProvider>());
}

NevaExtensionsClient::~NevaExtensionsClient() {}

void NevaExtensionsClient::Initialize() {
  // TODO(jamescook): Do we need to whitelist any extensions?
}

void NevaExtensionsClient::InitializeWebStoreUrls(
    base::CommandLine* command_line) {}

const extensions::PermissionMessageProvider&
NevaExtensionsClient::GetPermissionMessageProvider() const {
  NOTIMPLEMENTED();
  return g_permission_message_provider.Get();
}

const std::string NevaExtensionsClient::GetProductName() {
  return "neva_chrome_extension";
}

void NevaExtensionsClient::FilterHostPermissions(
    const extensions::URLPatternSet& hosts,
    extensions::URLPatternSet* new_hosts,
    extensions::PermissionIDSet* permissions) const {
  NOTIMPLEMENTED();
}

void NevaExtensionsClient::SetScriptingAllowlist(
    const extensions::ExtensionsClient::ScriptingAllowlist& allowlist) {
  scripting_allowlist_ = allowlist;
}

const extensions::ExtensionsClient::ScriptingAllowlist&
NevaExtensionsClient::GetScriptingAllowlist() const {
  // TODO(jamescook): Real allowlist.
  return scripting_allowlist_;
}

extensions::URLPatternSet NevaExtensionsClient::GetPermittedChromeSchemeHosts(
    const extensions::Extension* extension,
    const extensions::APIPermissionSet& api_permissions) const {
  NOTIMPLEMENTED();
  return extensions::URLPatternSet();
}

bool NevaExtensionsClient::IsScriptableURL(const GURL& url,
                                           std::string* error) const {
  // No restrictions on URLs.
  return true;
}

const GURL& NevaExtensionsClient::GetWebstoreBaseURL() const {
  return webstore_base_url_;
}

const GURL& NevaExtensionsClient::GetNewWebstoreBaseURL() const {
  return new_webstore_base_url_;
}

const GURL& NevaExtensionsClient::GetWebstoreUpdateURL() const {
  return webstore_update_url_;
}

bool NevaExtensionsClient::IsBlocklistUpdateURL(const GURL& url) const {
  return false;
}

}  // namespace neva
