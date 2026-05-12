// Copyright 2024 LG Electronics, Inc.
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

#include "neva/extensions/browser/api/web_navigation/web_navigation_api_helpers.h"

#include <memory>

#include "content/public/browser/navigation_handle.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/web_contents.h"
#include "extensions/browser/event_router.h"
#include "extensions/browser/extension_api_frame_id_map.h"
#include "extensions/common/mojom/event_dispatcher.mojom.h"
#include "neva/extensions/browser/api/web_navigation/web_navigation_api_constants.h"
#include "neva/extensions/browser/neva_extensions_service_factory.h"
#include "neva/extensions/browser/neva_extensions_service_impl.h"
#include "neva/extensions/browser/tab_helper.h"
#include "neva/extensions/common/api/web_navigation.h"

namespace web_navigation = extensions::api::web_navigation;

namespace neva {

namespace web_navigation_api_helpers {

namespace {

// Dispatches events to the extension message service.
void DispatchEvent(content::BrowserContext* browser_context,
                   std::unique_ptr<extensions::Event> event,
                   const GURL& url) {
  extensions::EventRouter* event_router =
      extensions::EventRouter::Get(browser_context);
  if (event_router) {
    extensions::mojom::EventFilteringInfoPtr info =
        extensions::mojom::EventFilteringInfo::New();
    info->url = url;
    // DCHECK_EQ(profile, event->restrict_to_browser_context);
    event->filter_info = std::move(info);
    event_router->BroadcastEvent(std::move(event));
  }
}

}  // namespace

void DispatchOnBeforeNavigate(content::NavigationHandle* navigation_handle) {
  content::WebContents* web_contents = navigation_handle->GetWebContents();
  GURL url(navigation_handle->GetURL());

  web_navigation::OnBeforeNavigate::Details details;
  details.tab_id = NevaExtensionsServiceFactory::GetService(
                       web_contents->GetBrowserContext())
                       ->GetTabHelper()
                       ->GetIdFromWebContents(web_contents);
  details.url = url.spec();
  details.process_id = -1;
  // There is no documentId for this event because the document has not
  // been created yet. It will first appear in the OnCommitted event. The
  // frameId can be used to associate OnBeforeNavigate and OnCommitted together.
  details.frame_id =
      extensions::ExtensionApiFrameIdMap::GetFrameId(navigation_handle);
  details.parent_frame_id =
      extensions::ExtensionApiFrameIdMap::GetParentFrameId(navigation_handle);
  // Only set the parentDocumentId value if we have a parent.
  if (content::RenderFrameHost* parent_frame_host =
          navigation_handle->GetParentFrameOrOuterDocument()) {
    details.parent_document_id =
        extensions::ExtensionApiFrameIdMap::GetDocumentId(parent_frame_host)
            .ToString();
  }
  details.frame_type =
      extensions::ExtensionApiFrameIdMap::GetFrameType(navigation_handle);
  details.document_lifecycle =
      extensions::ExtensionApiFrameIdMap::GetDocumentLifecycle(
          navigation_handle);
  details.time_stamp = base::Time::Now().InMillisecondsFSinceUnixEpoch();

  auto event = std::make_unique<extensions::Event>(
      extensions::events::WEB_NAVIGATION_ON_BEFORE_NAVIGATE,
      web_navigation::OnBeforeNavigate::kEventName,
      web_navigation::OnBeforeNavigate::Create(details),
      navigation_handle->GetWebContents()->GetBrowserContext());

  extensions::mojom::EventFilteringInfoPtr info =
      extensions::mojom::EventFilteringInfo::New();
  info->url = navigation_handle->GetURL();
  event->filter_info = std::move(info);

  content::BrowserContext* browser_context = web_contents->GetBrowserContext();
  DispatchEvent(browser_context, std::move(event), url);
}

// Constructs and dispatches an onCommitted or onHistoryStateUpdated event.
void DispatchOnCommitted(extensions::events::HistogramValue histogram_value,
                         const std::string& event_name,
                         content::NavigationHandle* navigation_handle) {
  content::WebContents* web_contents = navigation_handle->GetWebContents();
  GURL url(navigation_handle->GetURL());
  content::RenderFrameHost* frame_host =
      navigation_handle->GetRenderFrameHost();
  ui::PageTransition transition_type = navigation_handle->GetPageTransition();

  base::Value::List args;
  base::Value::Dict dict;
  int tab_id = static_cast<int>(NevaExtensionsServiceFactory::GetService(
                                    web_contents->GetBrowserContext())
                                    ->GetTabHelper()
                                    ->GetIdFromWebContents(web_contents));
  dict.Set(web_navigation_api_constants::kTabIdKey, tab_id);
  dict.Set(web_navigation_api_constants::kUrlKey, url.spec());
  dict.Set(web_navigation_api_constants::kProcessIdKey,
           frame_host->GetProcess()->GetID());
  dict.Set(web_navigation_api_constants::kFrameIdKey,
           extensions::ExtensionApiFrameIdMap::GetFrameId(frame_host));
  dict.Set(web_navigation_api_constants::kParentFrameIdKey,
           extensions::ExtensionApiFrameIdMap::GetParentFrameId(frame_host));
  dict.Set(
      web_navigation_api_constants::kDocumentIdKey,
      extensions::ExtensionApiFrameIdMap::GetDocumentId(frame_host).ToString());
  // Only set the parentDocumentId value if we have a parent.
  if (content::RenderFrameHost* parent_frame_host =
          frame_host->GetParentOrOuterDocument()) {
    dict.Set(
        web_navigation_api_constants::kParentDocumentIdKey,
        extensions::ExtensionApiFrameIdMap::GetDocumentId(parent_frame_host)
            .ToString());
  }
  dict.Set(
      web_navigation_api_constants::kFrameTypeKey,
      ToString(extensions::ExtensionApiFrameIdMap::GetFrameType(frame_host)));
  dict.Set(web_navigation_api_constants::kDocumentLifecycleKey,
           ToString(extensions::ExtensionApiFrameIdMap::GetDocumentLifecycle(
               frame_host)));

  if (navigation_handle->WasServerRedirect()) {
    transition_type = ui::PageTransitionFromInt(
        transition_type | ui::PAGE_TRANSITION_SERVER_REDIRECT);
  }

  std::string transition_type_string =
      ui::PageTransitionGetCoreTransitionString(transition_type);
  // For webNavigation API backward compatibility, keep "start_page" even after
  // renamed to "auto_toplevel".
  if (ui::PageTransitionCoreTypeIs(transition_type,
                                   ui::PAGE_TRANSITION_AUTO_TOPLEVEL)) {
    transition_type_string = "start_page";
  }
  dict.Set(web_navigation_api_constants::kTransitionTypeKey,
           transition_type_string);
  base::Value::List qualifiers;
  if (transition_type & ui::PAGE_TRANSITION_CLIENT_REDIRECT) {
    qualifiers.Append("client_redirect");
  }
  if (transition_type & ui::PAGE_TRANSITION_SERVER_REDIRECT) {
    qualifiers.Append("server_redirect");
  }
  if (transition_type & ui::PAGE_TRANSITION_FORWARD_BACK) {
    qualifiers.Append("forward_back");
  }
  if (transition_type & ui::PAGE_TRANSITION_FROM_ADDRESS_BAR) {
    qualifiers.Append("from_address_bar");
  }
  dict.Set(web_navigation_api_constants::kTransitionQualifiersKey,
           std::move(qualifiers));
  dict.Set(web_navigation_api_constants::kTimeStampKey,
           base::Time::Now().InMillisecondsFSinceUnixEpoch());
  args.Append(std::move(dict));

  content::BrowserContext* browser_context =
      navigation_handle->GetWebContents()->GetBrowserContext();
  auto event = std::make_unique<extensions::Event>(
      histogram_value, event_name, std::move(args), browser_context);
  DispatchEvent(browser_context, std::move(event), url);
}

// Constructs and dispatches an onDOMContentLoaded event.
void DispatchOnDOMContentLoaded(content::WebContents* web_contents,
                                content::RenderFrameHost* frame_host,
                                const GURL& url) {
  web_navigation::OnDOMContentLoaded::Details details;
  content::BrowserContext* browser_context = web_contents->GetBrowserContext();
  details.tab_id = NevaExtensionsServiceFactory::GetService(browser_context)
                       ->GetTabHelper()
                       ->GetIdFromWebContents(web_contents);
  details.url = url.spec();
  details.process_id = frame_host->GetProcess()->GetID();
  details.frame_id = extensions::ExtensionApiFrameIdMap::GetFrameId(frame_host);
  details.parent_frame_id =
      extensions::ExtensionApiFrameIdMap::GetParentFrameId(frame_host);
  details.document_id =
      extensions::ExtensionApiFrameIdMap::GetDocumentId(frame_host).ToString();
  // Only set the parentDocumentId value if we have a parent.
  if (content::RenderFrameHost* parent_frame_host =
          frame_host->GetParentOrOuterDocument()) {
    details.parent_document_id =
        extensions::ExtensionApiFrameIdMap::GetDocumentId(parent_frame_host)
            .ToString();
  }
  details.frame_type =
      extensions::ExtensionApiFrameIdMap::GetFrameType(frame_host);
  details.document_lifecycle =
      extensions::ExtensionApiFrameIdMap::GetDocumentLifecycle(frame_host);
  details.time_stamp = base::Time::Now().InMillisecondsFSinceUnixEpoch();

  auto event = std::make_unique<extensions::Event>(
      extensions::events::WEB_NAVIGATION_ON_DOM_CONTENT_LOADED,
      web_navigation::OnDOMContentLoaded::kEventName,
      web_navigation::OnDOMContentLoaded::Create(details), browser_context);
  DispatchEvent(browser_context, std::move(event), url);
}

// Constructs and dispatches an onCompleted event.
void DispatchOnCompleted(content::WebContents* web_contents,
                         content::RenderFrameHost* frame_host,
                         const GURL& url) {
  web_navigation::OnCompleted::Details details;
  content::BrowserContext* browser_context = web_contents->GetBrowserContext();
  details.tab_id = NevaExtensionsServiceFactory::GetService(browser_context)
                       ->GetTabHelper()
                       ->GetIdFromWebContents(web_contents);
  details.url = url.spec();
  details.process_id = frame_host->GetProcess()->GetID();
  details.frame_id = extensions::ExtensionApiFrameIdMap::GetFrameId(frame_host);
  details.parent_frame_id =
      extensions::ExtensionApiFrameIdMap::GetParentFrameId(frame_host);
  details.document_id =
      extensions::ExtensionApiFrameIdMap::GetDocumentId(frame_host).ToString();
  // Only set the parentDocumentId value if we have a parent.
  if (content::RenderFrameHost* parent_frame_host =
          frame_host->GetParentOrOuterDocument()) {
    details.parent_document_id =
        extensions::ExtensionApiFrameIdMap::GetDocumentId(parent_frame_host)
            .ToString();
  }
  details.frame_type =
      extensions::ExtensionApiFrameIdMap::GetFrameType(frame_host);
  details.document_lifecycle =
      extensions::ExtensionApiFrameIdMap::GetDocumentLifecycle(frame_host);
  details.time_stamp = base::Time::Now().InMillisecondsFSinceUnixEpoch();

  auto event = std::make_unique<extensions::Event>(
      extensions::events::WEB_NAVIGATION_ON_COMPLETED,
      web_navigation::OnCompleted::kEventName,
      web_navigation::OnCompleted::Create(details), browser_context);
  DispatchEvent(browser_context, std::move(event), url);
}

// Constructs and dispatches an onCreatedNavigationTarget event.
void DispatchOnCreatedNavigationTarget(
    int source_tab_id,
    int source_render_process_id,
    int source_extension_frame_id,
    content::BrowserContext* browser_context,
    content::WebContents* target_web_contents,
    const GURL& target_url) {
  web_navigation::OnCreatedNavigationTarget::Details details;
  details.source_tab_id = source_tab_id;
  details.source_process_id = source_render_process_id;
  details.source_frame_id = source_extension_frame_id;
  details.url = target_url.possibly_invalid_spec();
  details.tab_id = NevaExtensionsServiceFactory::GetService(browser_context)
                       ->GetTabHelper()
                       ->GetIdFromWebContents(target_web_contents);
  details.time_stamp = base::Time::Now().InMillisecondsFSinceUnixEpoch();

  auto event = std::make_unique<extensions::Event>(
      extensions::events::WEB_NAVIGATION_ON_CREATED_NAVIGATION_TARGET,
      web_navigation::OnCreatedNavigationTarget::kEventName,
      web_navigation::OnCreatedNavigationTarget::Create(details),
      browser_context);
  DispatchEvent(browser_context, std::move(event), target_url);
}

// Constructs and dispatches an onErrorOccurred event.
void DispatchOnErrorOccurred(content::WebContents* web_contents,
                             content::RenderFrameHost* frame_host,
                             const GURL& url,
                             int error_code) {
  web_navigation::OnErrorOccurred::Details details;
  content::BrowserContext* browser_context = web_contents->GetBrowserContext();
  details.tab_id = NevaExtensionsServiceFactory::GetService(browser_context)
                       ->GetTabHelper()
                       ->GetIdFromWebContents(web_contents);
  details.url = url.spec();
  details.process_id = frame_host->GetProcess()->GetID();
  details.frame_id = extensions::ExtensionApiFrameIdMap::GetFrameId(frame_host);
  details.parent_frame_id =
      extensions::ExtensionApiFrameIdMap::GetParentFrameId(frame_host);
  details.error = net::ErrorToString(error_code);
  details.document_id =
      extensions::ExtensionApiFrameIdMap::GetDocumentId(frame_host).ToString();
  // Only set the parentDocumentId value if we have a parent.
  if (content::RenderFrameHost* parent_frame_host =
          frame_host->GetParentOrOuterDocument()) {
    details.parent_document_id =
        extensions::ExtensionApiFrameIdMap::GetDocumentId(parent_frame_host)
            .ToString();
  }
  details.frame_type =
      extensions::ExtensionApiFrameIdMap::GetFrameType(frame_host);
  details.document_lifecycle =
      extensions::ExtensionApiFrameIdMap::GetDocumentLifecycle(frame_host);
  details.time_stamp = base::Time::Now().InMillisecondsFSinceUnixEpoch();

  auto event = std::make_unique<extensions::Event>(
      extensions::events::WEB_NAVIGATION_ON_ERROR_OCCURRED,
      web_navigation::OnErrorOccurred::kEventName,
      web_navigation::OnErrorOccurred::Create(details), browser_context);
  DispatchEvent(browser_context, std::move(event), url);
}

void DispatchOnErrorOccurred(content::NavigationHandle* navigation_handle) {
  web_navigation::OnErrorOccurred::Details details;
  content::BrowserContext* browser_context =
      navigation_handle->GetWebContents()->GetBrowserContext();
  details.tab_id =
      NevaExtensionsServiceFactory::GetService(browser_context)
          ->GetTabHelper()
          ->GetIdFromWebContents(navigation_handle->GetWebContents());
  details.url = navigation_handle->GetURL().spec();
  details.process_id = -1;
  details.frame_id =
      extensions::ExtensionApiFrameIdMap::GetFrameId(navigation_handle);
  details.parent_frame_id =
      extensions::ExtensionApiFrameIdMap::GetParentFrameId(navigation_handle);
  details.error = (navigation_handle->GetNetErrorCode() != net::OK)
                      ? net::ErrorToString(navigation_handle->GetNetErrorCode())
                      : net::ErrorToString(net::ERR_ABORTED);
  details.document_id =
      extensions::ExtensionApiFrameIdMap::GetDocumentId(navigation_handle)
          .ToString();
  // Only set the parentDocumentId value if we have a parent.
  if (content::RenderFrameHost* parent_frame_host =
          navigation_handle->GetParentFrameOrOuterDocument()) {
    details.parent_document_id =
        extensions::ExtensionApiFrameIdMap::GetDocumentId(parent_frame_host)
            .ToString();
  }
  details.frame_type =
      extensions::ExtensionApiFrameIdMap::GetFrameType(navigation_handle);
  details.document_lifecycle =
      extensions::ExtensionApiFrameIdMap::GetDocumentLifecycle(
          navigation_handle);
  details.time_stamp = base::Time::Now().InMillisecondsFSinceUnixEpoch();

  auto event = std::make_unique<extensions::Event>(
      extensions::events::WEB_NAVIGATION_ON_ERROR_OCCURRED,
      web_navigation::OnErrorOccurred::kEventName,
      web_navigation::OnErrorOccurred::Create(details), browser_context);
  DispatchEvent(browser_context, std::move(event), navigation_handle->GetURL());
}

}  // namespace web_navigation_api_helpers

}  // namespace neva
