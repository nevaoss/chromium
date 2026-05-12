// Copyright 2023 LG Electronics, Inc.
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

#include "neva/extensions/browser/api/messaging/neva_messaging_delegate.h"

#include <optional>

#include "content/public/browser/web_contents.h"
#include "extensions/browser/api/messaging/extension_message_port.h"
#include "extensions/browser/extension_api_frame_id_map.h"
#include "neva/extensions/browser/neva_extensions_service_factory.h"
#include "neva/extensions/browser/neva_extensions_service_impl.h"
#include "neva/extensions/browser/tab_helper.h"

namespace neva {

NevaMessagingDelegate::NevaMessagingDelegate() = default;
NevaMessagingDelegate::~NevaMessagingDelegate() = default;

std::optional<base::Value::Dict> NevaMessagingDelegate::MaybeGetTabInfo(
    content::WebContents* web_contents) {
  // Add info about the opener's tab (if it was a tab).
  if (!web_contents) {
    return std::nullopt;
  }

  TabHelper* tab_helper = NevaExtensionsServiceFactory::GetService(
                              web_contents->GetBrowserContext())
                              ->GetTabHelper();

  if (tab_helper) {
    // Give only id and url from the Tab properties(tabs.json), at the moment.
    base::Value::Dict tab_info;
    tab_info.Set(
        "id", static_cast<int>(tab_helper->GetIdFromWebContents(web_contents)));
    tab_info.Set("url", web_contents->GetLastCommittedURL().spec());
    return tab_info;
  }

  return std::nullopt;
}

content::WebContents* NevaMessagingDelegate::GetWebContentsByTabId(
    content::BrowserContext* browser_context,
    int tab_id) {
  TabHelper* tab_helper =
      NevaExtensionsServiceFactory::GetService(browser_context)->GetTabHelper();

  if (tab_helper) {
    return tab_helper->GetWebContentsFromId(tab_id);
  }

  return nullptr;
}

std::unique_ptr<extensions::MessagePort>
NevaMessagingDelegate::CreateReceiverForTab(
    base::WeakPtr<extensions::MessagePort::ChannelDelegate> channel_delegate,
    const std::string& extension_id,
    const extensions::PortId& receiver_port_id,
    content::WebContents* receiver_contents,
    int receiver_frame_id,
    const std::string& receiver_document_id) {
  // Frame ID -1 is every frame in the tab.
  bool include_child_frames =
      receiver_frame_id == -1 && receiver_document_id.empty();

  content::RenderFrameHost* receiver_rfh = nullptr;
  if (include_child_frames) {
    // The target is the active outermost main frame of the WebContents.
    receiver_rfh = receiver_contents->GetPrimaryMainFrame();
  } else if (!receiver_document_id.empty()) {
    extensions::ExtensionApiFrameIdMap::DocumentId document_id =
        extensions::ExtensionApiFrameIdMap::DocumentIdFromString(
            receiver_document_id);

    // Return early for invalid documentIds.
    if (!document_id) {
      return nullptr;
    }

    receiver_rfh = extensions::ExtensionApiFrameIdMap::Get()
                       ->GetRenderFrameHostByDocumentId(document_id);

    // If both |document_id| and |receiver_frame_id| are provided they
    // should find the same RenderFrameHost, if not return early.
    if (receiver_frame_id != -1 &&
        extensions::ExtensionApiFrameIdMap::GetRenderFrameHostById(
            receiver_contents, receiver_frame_id) != receiver_rfh) {
      return nullptr;
    }
  } else {
    DCHECK_GT(receiver_frame_id, -1);
    receiver_rfh = extensions::ExtensionApiFrameIdMap::GetRenderFrameHostById(
        receiver_contents, receiver_frame_id);
  }
  if (!receiver_rfh) {
    return nullptr;
  }

  return std::make_unique<extensions::ExtensionMessagePort>(
      channel_delegate, receiver_port_id, extension_id, receiver_rfh,
      include_child_frames);
}

}  // namespace neva
