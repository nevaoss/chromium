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

#ifndef NEVA_EXTENSIONS_COMMON_NEVA_EXTENSIONS_CLIENT_H_
#define NEVA_EXTENSIONS_COMMON_NEVA_EXTENSIONS_CLIENT_H_

#include "extensions/common/extensions_client.h"
#include "url/gurl.h"

namespace neva {

class NevaExtensionsClient : public extensions::ExtensionsClient {
 public:
  NevaExtensionsClient();
  NevaExtensionsClient(const NevaExtensionsClient&) = delete;
  NevaExtensionsClient& operator=(const NevaExtensionsClient&) = delete;
  ~NevaExtensionsClient() override;

  // extensions::ExtensionsClient overrides:
  void Initialize() override;
  void InitializeWebStoreUrls(base::CommandLine* command_line) override;
  const extensions::PermissionMessageProvider& GetPermissionMessageProvider()
      const override;
  const std::string GetProductName() override;
  void FilterHostPermissions(
      const extensions::URLPatternSet& hosts,
      extensions::URLPatternSet* new_hosts,
      extensions::PermissionIDSet* permissions) const override;
  void SetScriptingAllowlist(
      const ExtensionsClient::ScriptingAllowlist& allowlist) override;
  const ExtensionsClient::ScriptingAllowlist& GetScriptingAllowlist()
      const override;
  extensions::URLPatternSet GetPermittedChromeSchemeHosts(
      const extensions::Extension* extension,
      const extensions::APIPermissionSet& api_permissions) const override;
  bool IsScriptableURL(const GURL& url, std::string* error) const override;
  const GURL& GetWebstoreBaseURL() const override;
  const GURL& GetNewWebstoreBaseURL() const override;
  const GURL& GetWebstoreUpdateURL() const override;
  bool IsBlocklistUpdateURL(const GURL& url) const override;

 private:
  ScriptingAllowlist scripting_allowlist_;

  const GURL webstore_base_url_;
  const GURL new_webstore_base_url_;
  const GURL webstore_update_url_;
};

}  // namespace neva

#endif  // NEVA_EXTENSIONS_COMMON_NEVA_EXTENSIONS_CLIENT_H_
