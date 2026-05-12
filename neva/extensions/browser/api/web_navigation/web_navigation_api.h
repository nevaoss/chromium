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

#ifndef NEVA_EXTENSIONS_BROWSER_API_WEB_NAVIGATION_WEB_NAVIGATION_API_H_
#define NEVA_EXTENSIONS_BROWSER_API_WEB_NAVIGATION_WEB_NAVIGATION_API_H_

#include <map>
#include <memory>

#include "extensions/browser/browser_context_keyed_api_factory.h"
#include "extensions/browser/event_router.h"
#include "extensions/browser/extension_function.h"
#include "neva/extensions/browser/tab_event_observer.h"
#include "neva/extensions/browser/web_contents_event_observer.h"

namespace neva {

// Tab contents observer that forwards navigation events to the event router.
class WebNavigationTabObserver : public WebContentsEventObserver,
                                 public TabEventObserver {
 public:
  WebNavigationTabObserver(const WebNavigationTabObserver&) = delete;
  WebNavigationTabObserver& operator=(const WebNavigationTabObserver&) = delete;

  WebNavigationTabObserver();
  ~WebNavigationTabObserver() override;

  // WebContentsEventObserver implementation.
  void WebContentsDestroyed(content::WebContents* web_contents) override;
  void RenderFrameDeleted(content::WebContents* web_contents,
                          content::RenderFrameHost* render_frame_host) override;
  void DidStartNavigation(
      content::NavigationHandle* navigation_handle) override;
  void DidFinishNavigation(
      content::NavigationHandle* navigation_handle) override;
  void DOMContentLoaded(content::WebContents* web_contents,
                        content::RenderFrameHost* render_frame_host) override;
  void DidFinishLoad(content::WebContents* web_contents,
                     content::RenderFrameHost* render_frame_host,
                     const GURL& validated_url) override;
  void DidFailLoad(content::WebContents* web_contents,
                   content::RenderFrameHost* render_frame_host,
                   const GURL& validated_url,
                   int error_code) override;
  void DidOpenRequestedURL(content::WebContents* source_web_contents,
                           content::WebContents* new_contents,
                           content::RenderFrameHost* source_render_frame_host,
                           const GURL& url,
                           const content::Referrer& referrer,
                           WindowOpenDisposition disposition,
                           ui::PageTransition transition,
                           bool started_from_context_menu,
                           bool renderer_initiated) override;

  // TabEventObserver implementation.
  void OnExtensionTabCreated(content::WebContents* web_contents) override;

 private:
  // Used to cache the information about newly created WebContents objects.
  struct PendingWebContents {
    // The Extensions API ID for the source tab.
    int source_tab_id = -1;
    // The source frame's RenderProcessHost ID.
    int source_render_process_id = -1;
    // The Extensions API ID for the source frame.
    int source_extension_frame_id = -1;
    const GURL target_url;
  };

  // Mapping pointers to WebContents objects to information about how they got
  // created.
  std::map<content::WebContents*, PendingWebContents> pending_web_contents_;

  void HandleCommit(content::NavigationHandle* navigation_handle);
  void HandleError(content::NavigationHandle* navigation_handle);
  void RecordNewWebContents(content::WebContents* source_web_contents,
                            int source_render_process_id,
                            int source_render_frame_id,
                            const GURL& target_url,
                            content::WebContents* target_web_contents);
};

// API function that returns the states of all frames in a given tab.
class WebNavigationGetAllFramesFunction : public ExtensionFunction {
  ~WebNavigationGetAllFramesFunction() override {}
  ResponseAction Run() override;
  DECLARE_EXTENSION_FUNCTION("webNavigation.getAllFrames",
                             WEBNAVIGATION_GETALLFRAMES)
};

class WebNavigationAPI : public extensions::BrowserContextKeyedAPI,
                         public extensions::EventRouter::Observer {
 public:
  explicit WebNavigationAPI(content::BrowserContext* context);

  WebNavigationAPI(const WebNavigationAPI&) = delete;
  WebNavigationAPI& operator=(const WebNavigationAPI&) = delete;

  ~WebNavigationAPI() override;

  // KeyedService implementation.
  void Shutdown() override;

  // BrowserContextKeyedAPI implementation.
  static extensions::BrowserContextKeyedAPIFactory<WebNavigationAPI>*
  GetFactoryInstance();

  // EventRouter::Observer implementation.
  void OnListenerAdded(const extensions::EventListenerInfo& details) override;

 private:
  friend class extensions::BrowserContextKeyedAPIFactory<WebNavigationAPI>;
  friend class WebNavigationTabObserver;

  raw_ptr<content::BrowserContext> browser_context_;

  std::unique_ptr<WebNavigationTabObserver> tab_observer_;

  // BrowserContextKeyedAPI implementation.
  static const char* service_name() { return "WebNavigationAPI"; }
};

}  // namespace neva

#endif  // NEVA_EXTENSIONS_BROWSER_API_WEB_NAVIGATION_WEB_NAVIGATION_API_H_
