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

#ifndef NEVA_EXTENSIONS_BROWSER_NEVA_EXTENSIONS_BROWSER_CLIENT_H_
#define NEVA_EXTENSIONS_BROWSER_NEVA_EXTENSIONS_BROWSER_CLIENT_H_

#include <map>

#include "extensions/browser/extensions_browser_client.h"
#include "extensions/browser/kiosk/kiosk_delegate.h"
#include "services/network/public/mojom/url_loader_factory.mojom-forward.h"

class PrefService;

namespace extensions {
class ExtensionsAPIClient;
class KioskDelegate;
}  // namespace extensions

namespace url {
class Origin;
}  // namespace url

namespace neva {

class NevaExtensionsBrowserClient : public extensions::ExtensionsBrowserClient {
 public:
  NevaExtensionsBrowserClient();
  NevaExtensionsBrowserClient(const NevaExtensionsBrowserClient&) = delete;
  NevaExtensionsBrowserClient& operator=(const NevaExtensionsBrowserClient&) =
      delete;
  ~NevaExtensionsBrowserClient() override;

  // extensions::ExtensionsBrowserClient overrides:
  void Init() override;
  bool IsShuttingDown() override;
  bool AreExtensionsDisabled(const base::CommandLine& command_line,
                             content::BrowserContext* context) override;
  bool IsValidContext(void* context) override;
  bool IsSameContext(content::BrowserContext* first,
                     content::BrowserContext* second) override;
  bool HasOffTheRecordContext(content::BrowserContext* context) override;
  content::BrowserContext* GetOffTheRecordContext(
      content::BrowserContext* context) override;
  content::BrowserContext* GetOriginalContext(
      content::BrowserContext* context) override;
  content::BrowserContext* GetContextRedirectedToOriginal(
      content::BrowserContext* context) override;
  content::BrowserContext* GetContextRedirectedToOriginalWithoutAshInternals(
      content::BrowserContext* context) override;
  content::BrowserContext* GetContextOwnInstance(
      content::BrowserContext* context) override;
  content::BrowserContext* GetContextForOriginalOnly(
      content::BrowserContext* context) override;
  bool AreExtensionsDisabledForContext(
      content::BrowserContext* context) override;
  bool IsGuestSession(content::BrowserContext* context) const override;
  bool IsExtensionIncognitoEnabled(
      const std::string& extension_id,
      content::BrowserContext* context) const override;
  bool CanExtensionCrossIncognito(
      const extensions::Extension* extension,
      content::BrowserContext* context) const override;
  base::FilePath GetBundleResourcePath(
      const network::ResourceRequest& request,
      const base::FilePath& extension_resources_path,
      int* resource_id) const override;
  void LoadResourceFromResourceBundle(
      const network::ResourceRequest& request,
      mojo::PendingReceiver<network::mojom::URLLoader> loader,
      const base::FilePath& resource_relative_path,
      int resource_id,
      scoped_refptr<net::HttpResponseHeaders> headers,
      mojo::PendingRemote<network::mojom::URLLoaderClient> client) override;
  bool AllowCrossRendererResourceLoad(
      const network::ResourceRequest& request,
      network::mojom::RequestDestination destination,
      ui::PageTransition page_transition,
      int child_id,
      bool is_incognito,
      const extensions::Extension* extension,
      const extensions::ExtensionSet& extensions,
      const extensions::ProcessMap& process_map,
      const GURL& upstream_url) override;
  void GetEarlyExtensionPrefsObservers(
      content::BrowserContext* context,
      std::vector<extensions::EarlyExtensionPrefsObserver*>* observers)
      const override;
  extensions::ProcessManagerDelegate* GetProcessManagerDelegate()
      const override;
  mojo::PendingRemote<network::mojom::URLLoaderFactory>
  GetControlledFrameEmbedderURLLoader(
      const url::Origin& app_origin,
      content::FrameTreeNodeId frame_tree_node_id,
      content::BrowserContext* browser_context) override;
  std::unique_ptr<extensions::ExtensionHostDelegate>
  CreateExtensionHostDelegate() override;
  bool DidVersionUpdate(content::BrowserContext* context) override;
  void PermitExternalProtocolHandler() override;
  bool IsInDemoMode() override;
  bool IsScreensaverInDemoMode(const std::string& app_id) override;
  bool IsRunningInForcedAppMode() override;
  bool IsAppModeForcedForApp(
      const extensions::ExtensionId& extension_id) override;
  bool IsLoggedInAsPublicAccount() override;
  extensions::ExtensionSystemProvider* GetExtensionSystemFactory() override;
  void RegisterBrowserInterfaceBindersForFrame(
      mojo::BinderMapWithContext<content::RenderFrameHost*>* binder_map,
      content::RenderFrameHost* render_frame_host,
      const extensions::Extension* extension) const override;
  std::unique_ptr<extensions::RuntimeAPIDelegate> CreateRuntimeAPIDelegate(
      content::BrowserContext* context) const override;
  const extensions::ComponentExtensionResourceManager*
  GetComponentExtensionResourceManager() override;
  void BroadcastEventToRenderers(
      extensions::events::HistogramValue histogram_value,
      const std::string& event_name,
      base::ListValue args,
      bool dispatch_to_off_the_record_profiles) override;
  extensions::ExtensionCache* GetExtensionCache() override;
  bool IsBackgroundUpdateAllowed() override;
  bool IsMinBrowserVersionSupported(const std::string& min_version) override;
  void CreateExtensionWebContentsObserver(
      content::WebContents* web_contents) override;
  extensions::ExtensionWebContentsObserver* GetExtensionWebContentsObserver(
      content::WebContents* web_contents) override;
  extensions::KioskDelegate* GetKioskDelegate() override;
  extensions::SafeBrowsingDelegate* GetSafeBrowsingDelegate() override;
  std::string GetApplicationLocale() override;

  // |context| is the single BrowserContext used for IsValidContext().
  // |pref_service| is used for GetPrefServiceForContext().
  void AssociateWithBrowserContext(content::BrowserContext* context,
                                   PrefService* pref_service);

 private:
  // Not owned. Must be initialized when ready by
  // calling AssociateWithBrowserContext().
  std::map<raw_ptr<content::BrowserContext>, raw_ptr<PrefService>>
      context_prefs_map_;

  // Support for extension APIs.
  std::unique_ptr<extensions::ExtensionsAPIClient> api_client_;

  std::unique_ptr<extensions::KioskDelegate> kiosk_delegate_;

  std::unique_ptr<extensions::SafeBrowsingDelegate> safe_browsing_delegate_;
};

}  // namespace neva

#endif  // NEVA_EXTENSIONS_BROWSER_NEVA_EXTENSIONS_BROWSER_CLIENT_H_
