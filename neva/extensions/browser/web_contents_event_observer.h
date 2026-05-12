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

#ifndef NEVA_EXTENSIONS_BROWSER_API_WEB_NAVIGATION_WEB_CONTENTS_ITEM_OBSERVER_H_
#define NEVA_EXTENSIONS_BROWSER_API_WEB_NAVIGATION_WEB_CONTENTS_ITEM_OBSERVER_H_

#include "content/public/browser/navigation_handle.h"
#include "ui/base/window_open_disposition.h"

namespace neva {

class WebContentsEventObserver {
 public:
  WebContentsEventObserver() = default;
  virtual ~WebContentsEventObserver() = default;

  virtual void WebContentsDestroyed(content::WebContents* web_contents) = 0;
  virtual void RenderFrameDeleted(
      content::WebContents* web_contents,
      content::RenderFrameHost* render_frame_host) = 0;
  virtual void DidStartNavigation(content::NavigationHandle* navigation_handle) = 0;
  virtual void DidFinishNavigation(
      content::NavigationHandle* navigation_handle) = 0;
  virtual void DOMContentLoaded(content::WebContents* web_contents,
                                content::RenderFrameHost* render_frame_host) = 0;
  virtual void DidFinishLoad(content::WebContents* web_contents,
                             content::RenderFrameHost* render_frame_host,
                             const GURL& validated_url) = 0;
  virtual void DidFailLoad(content::WebContents* web_contents,
                           content::RenderFrameHost* render_frame_host,
                           const GURL& validated_url,
                           int error_code) = 0;
  virtual void DidOpenRequestedURL(
      content::WebContents* source_web_contents,
      content::WebContents* new_contents,
      content::RenderFrameHost* source_render_frame_host,
      const GURL& url,
      const content::Referrer& referrer,
      WindowOpenDisposition disposition,
      ui::PageTransition transition,
      bool started_from_context_menu,
      bool renderer_initiated) = 0;
};

}  // namespace neva

#endif  // NEVA_EXTENSIONS_BROWSER_API_WEB_NAVIGATION_WEB_CONTENTS_ITEM_OBSERVER_H_
