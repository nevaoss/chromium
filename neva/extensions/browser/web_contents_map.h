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

#ifndef NEVA_EXTENSIONS_BROWSER_WEB_CONTENTS_MAP_H
#define NEVA_EXTENSIONS_BROWSER_WEB_CONTENTS_MAP_H

#include <memory>
#include <set>

#include "base/memory/raw_ptr.h"
#include "base/memory/singleton.h"
#include "content/public/browser/web_contents.h"
#include "extensions/browser/extension_web_contents_observer.h"
#include "extensions/browser/script_executor.h"
#include "neva/extensions/browser/web_contents_event_observer.h"
#include "ui/base/window_open_disposition.h"
#include "url/gurl.h"

namespace neva {

class WebContentsMap;

class WebContentsItem : public extensions::ExtensionWebContentsObserver {
 public:
  WebContentsItem(WebContentsMap* web_contents_map,
                  content::WebContents* web_contents);
  ~WebContentsItem() override = default;

  void RenderFrameCreated(content::RenderFrameHost* render_frame_host) override;

  // content::WebContentsObserver implementation.
  void WebContentsDestroyed() override;
  void RenderFrameDeleted(content::RenderFrameHost* render_frame_host) override;
  void DidStartNavigation(
      content::NavigationHandle* navigation_handle) override;
  void DidFinishNavigation(
      content::NavigationHandle* navigation_handle) override;
  void DOMContentLoaded(content::RenderFrameHost* render_frame_host) override;
  void DidFinishLoad(content::RenderFrameHost* render_frame_host,
                     const GURL& validated_url) override;
  void DidFailLoad(content::RenderFrameHost* render_frame_host,
                   const GURL& validated_url,
                   int error_code) override;
  void DidOpenRequestedURL(content::WebContents* new_contents,
                           content::RenderFrameHost* source_render_frame_host,
                           const GURL& url,
                           const content::Referrer& referrer,
                           WindowOpenDisposition disposition,
                           ui::PageTransition transition,
                           bool started_from_context_menu,
                           bool renderer_initiated) override;

 private:
  raw_ptr<WebContentsMap> web_contents_map_;
};

class WebContentsMap {
 public:
  static WebContentsMap* GetInstance();

  void OnWebContentsCreated(content::WebContents* web_contents);

  void OnWebContentsWillDestroyed(WebContentsItem* item);

  extensions::ExtensionWebContentsObserver* GetObserver(
      content::WebContents* web_contents);

  void SetTabIdForRenderFrame(content::RenderFrameHost* host);

  void ForEachExtensionWebContents(
      const base::RepeatingCallback<void(content::WebContents*)>& cb);

  std::map<WebContentsItem*, content::WebContents*>::const_iterator begin()
      const {
    return items_.cbegin();
  }

  std::map<WebContentsItem*, content::WebContents*>::const_iterator end()
      const {
    return items_.cend();
  }

  extensions::ScriptExecutor* GetScriptExecutor(
      content::WebContents* web_contents);

  void RegisterEventObserver(WebContentsEventObserver* observer);

  void UnregisterEventObserver(WebContentsEventObserver* observer);

  std::vector<WebContentsEventObserver*>& GetEventObservers();

 private:
  friend struct base::DefaultSingletonTraits<WebContentsMap>;
  WebContentsMap();
  ~WebContentsMap();

  std::map<WebContentsItem*, content::WebContents*> items_;
  std::map<content::WebContents*, std::unique_ptr<extensions::ScriptExecutor>>
      script_executors_;
  std::vector<WebContentsEventObserver*> event_observers_;
};

}  // namespace neva

#endif  // NEVA_EXTENSIONS_BROWSER_WEB_CONTENTS_MAP_H
