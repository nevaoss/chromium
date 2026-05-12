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

#include "neva/extensions/browser/api/web_navigation/web_navigation_api.h"

#include <optional>

#include "base/lazy_instance.h"
#include "base/values.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "extensions/browser/event_router.h"
#include "extensions/browser/extension_api_frame_id_map.h"
#include "neva/extensions/browser/api/web_navigation/frame_navigation_state.h"
#include "neva/extensions/browser/api/web_navigation/web_navigation_api_helpers.h"
#include "neva/extensions/browser/neva_extensions_service_factory.h"
#include "neva/extensions/browser/neva_extensions_service_impl.h"
#include "neva/extensions/browser/tab_helper.h"
#include "neva/extensions/browser/web_contents_map.h"
#include "neva/extensions/common/api/web_navigation.h"

namespace neva {

namespace GetAllFrames = extensions::api::web_navigation::GetAllFrames;

namespace web_navigation = extensions::api::web_navigation;

WebNavigationTabObserver::WebNavigationTabObserver() = default;

WebNavigationTabObserver::~WebNavigationTabObserver() = default;

void WebNavigationTabObserver::WebContentsDestroyed(
    content::WebContents* web_contents) {
  pending_web_contents_.erase(web_contents);
}

void WebNavigationTabObserver::RenderFrameDeleted(
    content::WebContents* web_contents,
    content::RenderFrameHost* render_frame_host) {
  auto* navigation_state =
      FrameNavigationState::GetForCurrentDocument(render_frame_host);
  if (navigation_state && navigation_state->CanSendEvents() &&
      !navigation_state->GetDocumentLoadCompleted()) {
    web_navigation_api_helpers::DispatchOnErrorOccurred(
        web_contents, render_frame_host, navigation_state->GetUrl(),
        net::ERR_ABORTED);
    navigation_state->SetErrorOccurredInFrame();
  }
}

void WebNavigationTabObserver::DidStartNavigation(
    content::NavigationHandle* navigation_handle) {
  if (navigation_handle->IsSameDocument() ||
      !FrameNavigationState::IsValidUrl(navigation_handle->GetURL())) {
    return;
  }

  const std::string& event_name = web_navigation::OnBeforeNavigate::kEventName;
  content::WebContents* web_contents = navigation_handle->GetWebContents();
  content::BrowserContext* browser_context = web_contents->GetBrowserContext();
  if (!extensions::EventRouter::Get(browser_context)
           ->HasEventListener(event_name)) {
    return;
  }

  // Only dispatch the onBeforeNavigate event if the associated WebContents
  // is already added.
  if (NevaExtensionsServiceFactory::GetService(browser_context)
          ->GetTabHelper()
          ->GetTabIdFromWebContents(web_contents)) {
    web_navigation_api_helpers::DispatchOnBeforeNavigate(navigation_handle);
  }
}

void WebNavigationTabObserver::DidFinishNavigation(
    content::NavigationHandle* navigation_handle) {
  if (navigation_handle->HasCommitted() && !navigation_handle->IsErrorPage()) {
    HandleCommit(navigation_handle);
    return;
  }

  HandleError(navigation_handle);
}

void WebNavigationTabObserver::DOMContentLoaded(
    content::WebContents* web_contents,
    content::RenderFrameHost* render_frame_host) {
  auto* navigation_state =
      FrameNavigationState::GetForCurrentDocument(render_frame_host);
  if (!navigation_state || !navigation_state->CanSendEvents()) {
    return;
  }

  navigation_state->SetParsingFinished();
  web_navigation_api_helpers::DispatchOnDOMContentLoaded(
      web_contents, render_frame_host, navigation_state->GetUrl());

  if (!navigation_state->GetDocumentLoadCompleted()) {
    return;
  }

  // The load might already have finished by the time we finished parsing. For
  // compatibility reasons, we artifically delay the load completed signal until
  // after parsing was completed.
  web_navigation_api_helpers::DispatchOnCompleted(
      web_contents, render_frame_host, navigation_state->GetUrl());
}

void WebNavigationTabObserver::DidFinishLoad(
    content::WebContents* web_contents,
    content::RenderFrameHost* render_frame_host,
    const GURL& validated_url) {
  auto* navigation_state =
      FrameNavigationState::GetForCurrentDocument(render_frame_host);
  // When showing replacement content, we might get load signals for frames
  // that weren't regularly loaded.
  if (!navigation_state) {
    return;
  }

  navigation_state->SetDocumentLoadCompleted();
  if (!navigation_state->CanSendEvents()) {
    return;
  }

  // A new navigation might have started before the old one completed.
  // Ignore the old navigation completion in that case.
  if (navigation_state->GetUrl() != validated_url) {
    return;
  }

  // The load might already have finished by the time we finished parsing. For
  // compatibility reasons, we artifically delay the load completed signal until
  // after parsing was completed.
  if (!navigation_state->GetParsingFinished()) {
    return;
  }
  web_navigation_api_helpers::DispatchOnCompleted(
      web_contents, render_frame_host, validated_url);
}

void WebNavigationTabObserver::DidFailLoad(
    content::WebContents* web_contents,
    content::RenderFrameHost* render_frame_host,
    const GURL& validated_url,
    int error_code) {
  auto* navigation_state =
      FrameNavigationState::GetForCurrentDocument(render_frame_host);
  // When showing replacement content, we might get load signals for frames
  // that weren't regularly loaded.
  if (!navigation_state) {
    return;
  }

  if (navigation_state->CanSendEvents()) {
    web_navigation_api_helpers::DispatchOnErrorOccurred(
        web_contents, render_frame_host, navigation_state->GetUrl(),
        error_code);
  }
  navigation_state->SetErrorOccurredInFrame();
}

void WebNavigationTabObserver::DidOpenRequestedURL(
    content::WebContents* source_web_contents,
    content::WebContents* new_contents,
    content::RenderFrameHost* source_render_frame_host,
    const GURL& url,
    const content::Referrer& referrer,
    WindowOpenDisposition disposition,
    ui::PageTransition transition,
    bool started_from_context_menu,
    bool renderer_initiated) {
  auto* navigation_state =
      FrameNavigationState::GetForCurrentDocument(source_render_frame_host);
  if (!navigation_state || !navigation_state->CanSendEvents()) {
    return;
  }

  // We only send the onCreatedNavigationTarget if we end up creating a new
  // window.
  if (disposition != WindowOpenDisposition::SINGLETON_TAB &&
      disposition != WindowOpenDisposition::NEW_FOREGROUND_TAB &&
      disposition != WindowOpenDisposition::NEW_BACKGROUND_TAB &&
      disposition != WindowOpenDisposition::NEW_POPUP &&
      disposition != WindowOpenDisposition::NEW_WINDOW &&
      disposition != WindowOpenDisposition::OFF_THE_RECORD) {
    return;
  }

  RecordNewWebContents(
      source_web_contents, source_render_frame_host->GetProcess()->GetID(),
      source_render_frame_host->GetRoutingID(), url, new_contents);
}

void WebNavigationTabObserver::RecordNewWebContents(
    content::WebContents* source_web_contents,
    int source_render_process_id,
    int source_render_frame_id,
    const GURL& target_url,
    content::WebContents* target_web_contents) {
  if (source_render_frame_id == 0) {
    return;
  }

  auto* frame_host = content::RenderFrameHost::FromID(source_render_process_id,
                                                      source_render_frame_id);
  auto* frame_navigation_state =
      FrameNavigationState::GetForCurrentDocument(frame_host);

  if (!frame_navigation_state || !frame_navigation_state->CanSendEvents()) {
    return;
  }

  int source_extension_frame_id =
      extensions::ExtensionApiFrameIdMap::GetFrameId(frame_host);
  int source_tab_id = NevaExtensionsServiceFactory::GetService(
                          source_web_contents->GetBrowserContext())
                          ->GetTabHelper()
                          ->GetTabIdFromWebContents(source_web_contents);
  int target_tab_id = NevaExtensionsServiceFactory::GetService(
                          source_web_contents->GetBrowserContext())
                          ->GetTabHelper()
                          ->GetIdFromWebContents(target_web_contents);
  if (target_tab_id < 1) {
    pending_web_contents_.emplace(
        target_web_contents,
        PendingWebContents{source_tab_id, source_render_process_id,
                           source_extension_frame_id, target_url});
  } else {
    web_navigation_api_helpers::DispatchOnCreatedNavigationTarget(
        source_tab_id, source_render_process_id, source_extension_frame_id,
        target_web_contents->GetBrowserContext(), target_web_contents,
        target_url);
  }
}

void WebNavigationTabObserver::OnExtensionTabCreated(
    content::WebContents* tab) {
  auto iter = pending_web_contents_.find(tab);
  if (iter == pending_web_contents_.end()) {
    return;
  }

  const PendingWebContents& pending_tab = iter->second;
  web_navigation_api_helpers::DispatchOnCreatedNavigationTarget(
      pending_tab.source_tab_id, pending_tab.source_render_process_id,
      pending_tab.source_extension_frame_id, tab->GetBrowserContext(), tab,
      pending_tab.target_url);
  pending_web_contents_.erase(iter);
}

void WebNavigationTabObserver::HandleCommit(
    content::NavigationHandle* navigation_handle) {
  FrameNavigationState::GetOrCreateForCurrentDocument(
      navigation_handle->GetRenderFrameHost())
      ->StartTrackingDocumentLoad(
          navigation_handle->GetURL(), navigation_handle->IsSameDocument(),
          navigation_handle->IsServedFromBackForwardCache(),
          /*is_error_page=*/false);

  extensions::events::HistogramValue histogram_value =
      extensions::events::UNKNOWN;
  std::string event_name;
  if (navigation_handle->IsSameDocument()) {
    histogram_value =
        extensions::events::WEB_NAVIGATION_ON_HISTORY_STATE_UPDATED;
    event_name = web_navigation::OnHistoryStateUpdated::kEventName;
  } else {
    histogram_value = extensions::events::WEB_NAVIGATION_ON_COMMITTED;
    event_name = web_navigation::OnCommitted::kEventName;
  }
  web_navigation_api_helpers::DispatchOnCommitted(histogram_value, event_name,
                                                  navigation_handle);

  if (navigation_handle->IsServedFromBackForwardCache()) {
    web_navigation_api_helpers::DispatchOnCompleted(
        navigation_handle->GetWebContents(),
        navigation_handle->GetRenderFrameHost(), navigation_handle->GetURL());
  }
}

void WebNavigationTabObserver::HandleError(
    content::NavigationHandle* navigation_handle) {
  if (navigation_handle->HasCommitted()) {
    FrameNavigationState::GetOrCreateForCurrentDocument(
        navigation_handle->GetRenderFrameHost())
        ->StartTrackingDocumentLoad(navigation_handle->GetURL(),
                                    navigation_handle->IsSameDocument(),
                                    /*is_from_back_forward_cache=*/false,
                                    /*is_error_page=*/true);
  }

  web_navigation_api_helpers::DispatchOnErrorOccurred(navigation_handle);
}

ExtensionFunction::ResponseAction WebNavigationGetAllFramesFunction::Run() {
  std::optional<GetAllFrames::Params> params =
      GetAllFrames::Params::Create(args());
  EXTENSION_FUNCTION_VALIDATE(params);
  int tab_id = params->details.tab_id;

  content::WebContents* web_contents =
      NevaExtensionsServiceFactory::GetService(browser_context())
          ->GetTabHelper()
          ->GetWebContentsFromId(tab_id);
  if (!web_contents) {
    return RespondNow(WithArguments(base::Value()));
  }

  std::vector<GetAllFrames::Results::DetailsType> result_list;

  // We currently do not expose back/forward cached frames in the GetAllFrames
  // API, but we do explicitly include prerendered frames.
  web_contents->ForEachRenderFrameHostWithAction([web_contents, &result_list](
                                                     content::RenderFrameHost*
                                                         render_frame_host) {
    // Don't expose inner WebContents for the getFrames API.
    if (content::WebContents::FromRenderFrameHost(render_frame_host) !=
        web_contents) {
      return content::RenderFrameHost::FrameIterationAction::kSkipChildren;
    }

    auto* navigation_state =
        FrameNavigationState::GetForCurrentDocument(render_frame_host);

    if (!navigation_state ||
        !FrameNavigationState::IsValidUrl(navigation_state->GetUrl())) {
      return content::RenderFrameHost::FrameIterationAction::kContinue;
    }

    // Skip back/forward cached frames.
    if (render_frame_host->IsInLifecycleState(
            content::RenderFrameHost::LifecycleState::kInBackForwardCache)) {
      return content::RenderFrameHost::FrameIterationAction::kSkipChildren;
    }

    GetAllFrames::Results::DetailsType frame;
    frame.url = navigation_state->GetUrl().spec();
    frame.frame_id =
        extensions::ExtensionApiFrameIdMap::GetFrameId(render_frame_host);
    frame.parent_frame_id =
        extensions::ExtensionApiFrameIdMap::GetParentFrameId(render_frame_host);
    frame.document_id =
        extensions::ExtensionApiFrameIdMap::GetDocumentId(render_frame_host)
            .ToString();
    // Only set the parentDocumentId value if we have a parent.
    if (content::RenderFrameHost* parent_frame_host =
            render_frame_host->GetParentOrOuterDocument()) {
      frame.parent_document_id =
          extensions::ExtensionApiFrameIdMap::GetDocumentId(parent_frame_host)
              .ToString();
    }
    frame.frame_type =
        extensions::ExtensionApiFrameIdMap::GetFrameType(render_frame_host);
    frame.document_lifecycle =
        extensions::ExtensionApiFrameIdMap::GetDocumentLifecycle(
            render_frame_host);
    frame.process_id = render_frame_host->GetProcess()->GetID();
    frame.error_occurred = navigation_state->GetErrorOccurredInFrame();
    result_list.push_back(std::move(frame));
    return content::RenderFrameHost::FrameIterationAction::kContinue;
  });

  return RespondNow(ArgumentList(GetAllFrames::Results::Create(result_list)));
}

WebNavigationAPI::WebNavigationAPI(content::BrowserContext* context)
    : browser_context_(context) {
  extensions::EventRouter* event_router =
      extensions::EventRouter::Get(browser_context_);
  event_router->RegisterObserver(this,
                                 web_navigation::OnBeforeNavigate::kEventName);
  event_router->RegisterObserver(this, web_navigation::OnCommitted::kEventName);
  event_router->RegisterObserver(this, web_navigation::OnCompleted::kEventName);
  event_router->RegisterObserver(
      this, web_navigation::OnCreatedNavigationTarget::kEventName);
  event_router->RegisterObserver(
      this, web_navigation::OnHistoryStateUpdated::kEventName);
}

WebNavigationAPI::~WebNavigationAPI() {}

void WebNavigationAPI::Shutdown() {
  extensions::EventRouter::Get(browser_context_)->UnregisterObserver(this);
  if (tab_observer_) {
    WebContentsMap::GetInstance()->UnregisterEventObserver(tab_observer_.get());
    NevaExtensionsServiceFactory::GetService(browser_context_)
        ->UnregisterTabEventObserver(tab_observer_.get());
  }
}

static base::LazyInstance<extensions::BrowserContextKeyedAPIFactory<
    WebNavigationAPI>>::DestructorAtExit g_web_navigation_api_factory =
    LAZY_INSTANCE_INITIALIZER;

// static
extensions::BrowserContextKeyedAPIFactory<WebNavigationAPI>*
WebNavigationAPI::GetFactoryInstance() {
  return g_web_navigation_api_factory.Pointer();
}

void WebNavigationAPI::OnListenerAdded(
    const extensions::EventListenerInfo& details) {
  tab_observer_ =
      std::unique_ptr<WebNavigationTabObserver>(new WebNavigationTabObserver());
  WebContentsMap::GetInstance()->RegisterEventObserver(tab_observer_.get());
  NevaExtensionsServiceFactory::GetService(browser_context_)
      ->RegisterTabEventObserver(tab_observer_.get());
  extensions::EventRouter::Get(browser_context_)->UnregisterObserver(this);
}

}  // namespace neva
