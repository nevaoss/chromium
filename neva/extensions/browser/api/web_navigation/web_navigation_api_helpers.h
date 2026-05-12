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

#ifndef NEVA_EXTENSIONS_BROWSER_API_WEB_NAVIGATION_WEB_NAVIGATION_API_HELPERS_H_
#define NEVA_EXTENSIONS_BROWSER_API_WEB_NAVIGATION_WEB_NAVIGATION_API_HELPERS_H_

#include <string>

#include "extensions/browser/extension_event_histogram_value.h"

namespace content {
class BrowserContext;
class NavigationHandle;
class RenderFrameHost;
class WebContents;
}  // namespace content

class GURL;

namespace neva {

struct Event;

namespace web_navigation_api_helpers {

void DispatchOnBeforeNavigate(content::NavigationHandle* navigation_handle);

void DispatchOnCommitted(extensions::events::HistogramValue histogram_value,
                         const std::string& event_name,
                         content::NavigationHandle* navigation_handle);

void DispatchOnDOMContentLoaded(content::WebContents* web_contents,
                                content::RenderFrameHost* frame_host,
                                const GURL& url);

void DispatchOnCompleted(content::WebContents* web_contents,
                         content::RenderFrameHost* frame_host,
                         const GURL& url);

void DispatchOnCreatedNavigationTarget(
    int source_tab_id,
    int source_render_process_id,
    int source_extension_frame_id,
    content::BrowserContext* browser_context,
    content::WebContents* target_web_contents,
    const GURL& target_url);

void DispatchOnErrorOccurred(content::WebContents* web_contents,
                             content::RenderFrameHost* frame_host,
                             const GURL& url,
                             int error_code);
void DispatchOnErrorOccurred(content::NavigationHandle* navigation_handle);

}  // namespace web_navigation_api_helpers

}  // namespace neva

#endif  // NEVA_EXTENSIONS_BROWSER_API_WEB_NAVIGATION_WEB_NAVIGATION_API_HELPERS_H_
