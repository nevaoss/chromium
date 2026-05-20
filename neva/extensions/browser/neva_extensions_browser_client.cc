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

#include "neva/extensions/browser/neva_extensions_browser_client.h"

#include <memory>
#include <utility>

#include "base/functional/bind.h"
#include "base/memory/ptr_util.h"
#include "build/build_config.h"
#include "components/version_info/version_info.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_frame_host.h"
#include "extensions/browser/api/core_extensions_browser_api_provider.h"
#include "extensions/browser/api/extensions_api_client.h"
#include "extensions/browser/event_router.h"
#include "extensions/browser/extensions_browser_interface_binders.h"
#include "extensions/browser/null_app_sorting.h"
#include "extensions/browser/safe_browsing_delegate.h"
#include "extensions/browser/updater/null_extension_cache.h"
#include "extensions/browser/url_request_util.h"
#include "extensions/common/features/feature_channel.h"
#include "net/http/http_response_headers.h"
#include "neva/extensions/browser/neva_extension_api_client.h"
#include "neva/extensions/browser/neva_extension_host_delegate.h"
#include "neva/extensions/browser/neva_extension_system_factory.h"
#include "neva/extensions/browser/neva_extensions_browser_api_provider.h"
#include "neva/extensions/browser/neva_extensions_kiosk_delegate.h"
#include "neva/extensions/browser/neva_runtime_api_delegate.h"
#include "neva/extensions/browser/web_contents_map.h"
#include "services/network/public/mojom/url_loader.mojom.h"
#include "url/origin.h"

using content::BrowserContext;
using content::BrowserThread;

namespace neva {

NevaExtensionsBrowserClient::NevaExtensionsBrowserClient()
    : api_client_(new NevaExtensionsAPIClient),
      safe_browsing_delegate_(
          std::make_unique<extensions::SafeBrowsingDelegate>()) {
  // app_shell does not have a concept of channel yet, so leave UNKNOWN to
  // enable all channel-dependent extension APIs.
  extensions::SetCurrentChannel(version_info::Channel::UNKNOWN);

  AddAPIProvider(
      std::make_unique<extensions::CoreExtensionsBrowserAPIProvider>());
  AddAPIProvider(std::make_unique<NevaExtensionsBrowserAPIProvider>());
}

NevaExtensionsBrowserClient::~NevaExtensionsBrowserClient() {}

void NevaExtensionsBrowserClient::Init() {}

bool NevaExtensionsBrowserClient::IsShuttingDown() {
  return false;
}

bool NevaExtensionsBrowserClient::AreExtensionsDisabled(
    const base::CommandLine& command_line,
    BrowserContext* context) {
  return false;
}

bool NevaExtensionsBrowserClient::IsValidContext(void* context) {
  return context_prefs_map_.find(static_cast<content::BrowserContext*>(
             context)) != context_prefs_map_.end();
}

bool NevaExtensionsBrowserClient::IsSameContext(BrowserContext* first,
                                                BrowserContext* second) {
  return first == second;
}

bool NevaExtensionsBrowserClient::HasOffTheRecordContext(
    BrowserContext* context) {
  return false;
}

BrowserContext* NevaExtensionsBrowserClient::GetOffTheRecordContext(
    BrowserContext* context) {
  // app_shell only supports a single context.
  return NULL;
}

BrowserContext* NevaExtensionsBrowserClient::GetOriginalContext(
    BrowserContext* context) {
  return context;
}

content::BrowserContext*
NevaExtensionsBrowserClient::GetContextRedirectedToOriginal(
    content::BrowserContext* context) {
  return context;
}

content::BrowserContext*
NevaExtensionsBrowserClient::GetContextRedirectedToOriginalWithoutAshInternals(
    content::BrowserContext* context) {
  return context;
}

content::BrowserContext* NevaExtensionsBrowserClient::GetContextOwnInstance(
    content::BrowserContext* context) {
  return context;
}

content::BrowserContext* NevaExtensionsBrowserClient::GetContextForOriginalOnly(
    content::BrowserContext* context) {
  return context;
}

bool NevaExtensionsBrowserClient::AreExtensionsDisabledForContext(
    content::BrowserContext* context) {
  return false;
}

bool NevaExtensionsBrowserClient::IsGuestSession(
    BrowserContext* context) const {
  return false;
}

bool NevaExtensionsBrowserClient::IsExtensionIncognitoEnabled(
    const std::string& extension_id,
    content::BrowserContext* context) const {
  return false;
}

bool NevaExtensionsBrowserClient::CanExtensionCrossIncognito(
    const extensions::Extension* extension,
    content::BrowserContext* context) const {
  return false;
}

base::FilePath NevaExtensionsBrowserClient::GetBundleResourcePath(
    const network::ResourceRequest& request,
    const base::FilePath& extension_resources_path,
    int* resource_id) const {
  *resource_id = 0;
  return base::FilePath();
}

void NevaExtensionsBrowserClient::LoadResourceFromResourceBundle(
    const network::ResourceRequest& request,
    mojo::PendingReceiver<network::mojom::URLLoader> loader,
    const base::FilePath& resource_relative_path,
    int resource_id,
    scoped_refptr<net::HttpResponseHeaders> headers,
    mojo::PendingRemote<network::mojom::URLLoaderClient> client) {
  NOTREACHED() << "Load resources from bundles not supported.";
}

bool NevaExtensionsBrowserClient::AllowCrossRendererResourceLoad(
    const network::ResourceRequest& request,
    network::mojom::RequestDestination destination,
    ui::PageTransition page_transition,
    int child_id,
    bool is_incognito,
    const extensions::Extension* extension,
    const extensions::ExtensionSet& extensions,
    const extensions::ProcessMap& process_map,
    const GURL& upstream_url) {
  bool allowed = false;
  if (extensions::url_request_util::AllowCrossRendererResourceLoad(
          request, destination, page_transition, child_id, is_incognito,
          extension, extensions, process_map, upstream_url, &allowed)) {
    return allowed;
  }

  // Couldn't determine if resource is allowed. Block the load.
  return false;
}

void NevaExtensionsBrowserClient::GetEarlyExtensionPrefsObservers(
    content::BrowserContext* context,
    std::vector<extensions::EarlyExtensionPrefsObserver*>* observers) const {}

extensions::ProcessManagerDelegate*
NevaExtensionsBrowserClient::GetProcessManagerDelegate() const {
  return NULL;
}

mojo::PendingRemote<network::mojom::URLLoaderFactory>
NevaExtensionsBrowserClient::GetControlledFrameEmbedderURLLoader(
    const url::Origin& app_origin,
    content::FrameTreeNodeId frame_tree_node_id,
    content::BrowserContext* browser_context) {
  return mojo::PendingRemote<network::mojom::URLLoaderFactory>();
}

std::unique_ptr<extensions::ExtensionHostDelegate>
NevaExtensionsBrowserClient::CreateExtensionHostDelegate() {
  return std::make_unique<NevaExtensionHostDelegate>();
}

bool NevaExtensionsBrowserClient::DidVersionUpdate(BrowserContext* context) {
  // TODO(jamescook): We might want to tell extensions when app_shell updates.
  return false;
}

void NevaExtensionsBrowserClient::PermitExternalProtocolHandler() {}

bool NevaExtensionsBrowserClient::IsInDemoMode() {
  return false;
}

bool NevaExtensionsBrowserClient::IsScreensaverInDemoMode(
    const std::string& app_id) {
  return false;
}

bool NevaExtensionsBrowserClient::IsRunningInForcedAppMode() {
  return false;
}

bool NevaExtensionsBrowserClient::IsAppModeForcedForApp(
    const extensions::ExtensionId& extension_id) {
  return false;
}

bool NevaExtensionsBrowserClient::IsLoggedInAsPublicAccount() {
  return false;
}

extensions::ExtensionSystemProvider*
NevaExtensionsBrowserClient::GetExtensionSystemFactory() {
  return NevaExtensionSystemFactory::GetInstance();
}

void NevaExtensionsBrowserClient::RegisterBrowserInterfaceBindersForFrame(
    mojo::BinderMapWithContext<content::RenderFrameHost*>* binder_map,
    content::RenderFrameHost* render_frame_host,
    const extensions::Extension* extension) const {
  PopulateExtensionFrameBinders(binder_map, render_frame_host, extension);
}

std::unique_ptr<extensions::RuntimeAPIDelegate>
NevaExtensionsBrowserClient::CreateRuntimeAPIDelegate(
    content::BrowserContext* context) const {
  return std::make_unique<NevaRuntimeAPIDelegate>(context);
}

const extensions::ComponentExtensionResourceManager*
NevaExtensionsBrowserClient::GetComponentExtensionResourceManager() {
  return NULL;
}

void NevaExtensionsBrowserClient::BroadcastEventToRenderers(
    extensions::events::HistogramValue histogram_value,
    const std::string& event_name,
    base::ListValue args,
    bool dispatch_to_off_the_record_profiles) {
  if (!BrowserThread::CurrentlyOn(BrowserThread::UI)) {
    content::GetUIThreadTaskRunner({})->PostTask(
        FROM_HERE,
        base::BindOnce(&NevaExtensionsBrowserClient::BroadcastEventToRenderers,
                       base::Unretained(this), histogram_value, event_name,
                       std::move(args), dispatch_to_off_the_record_profiles));
    return;
  }

  auto event = std::make_unique<extensions::Event>(histogram_value, event_name,
                                                   std::move(args));
  for (auto& it : context_prefs_map_) {
    extensions::EventRouter::Get(it.first)->BroadcastEvent(std::move(event));
  }
}

extensions::ExtensionCache* NevaExtensionsBrowserClient::GetExtensionCache() {
  return nullptr;
}

bool NevaExtensionsBrowserClient::IsBackgroundUpdateAllowed() {
  return true;
}

bool NevaExtensionsBrowserClient::IsMinBrowserVersionSupported(
    const std::string& min_version) {
  return true;
}

void NevaExtensionsBrowserClient::CreateExtensionWebContentsObserver(
    content::WebContents* web_contents) {
  WebContentsMap::GetInstance()->OnWebContentsCreated(web_contents);
}

extensions::ExtensionWebContentsObserver*
NevaExtensionsBrowserClient::GetExtensionWebContentsObserver(
    content::WebContents* web_contents) {
  return WebContentsMap::GetInstance()->GetObserver(web_contents);
}

extensions::KioskDelegate* NevaExtensionsBrowserClient::GetKioskDelegate() {
  if (!kiosk_delegate_) {
    kiosk_delegate_ = std::make_unique<NevaExtensionsKioskDelegate>();
  }
  return kiosk_delegate_.get();
}

extensions::SafeBrowsingDelegate*
NevaExtensionsBrowserClient::GetSafeBrowsingDelegate() {
  return safe_browsing_delegate_.get();
}

std::string NevaExtensionsBrowserClient::GetApplicationLocale() {
  // TODO(michaelpg): Use system locale.
  return "en-US";
}

void NevaExtensionsBrowserClient::AssociateWithBrowserContext(
    content::BrowserContext* context,
    PrefService* pref_service) {
  if (context_prefs_map_.find(context) == context_prefs_map_.end()) {
    context_prefs_map_.emplace(context, pref_service);
  }
}

}  // namespace neva
