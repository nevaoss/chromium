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

#ifndef NEVA_EXTENSIONS_BROWSER_API_MESSAGING_NEVA_MESSAGING_DELEGATE_H_
#define NEVA_EXTENSIONS_BROWSER_API_MESSAGING_NEVA_MESSAGING_DELEGATE_H_

#include <optional>

#include "extensions/browser/api/messaging/messaging_delegate.h"

namespace neva {

// Helper class for features of the extension messaging API.
class NevaMessagingDelegate : public extensions::MessagingDelegate {
 public:
  NevaMessagingDelegate();

  NevaMessagingDelegate(const NevaMessagingDelegate&) = delete;
  NevaMessagingDelegate& operator=(const NevaMessagingDelegate&) = delete;

  ~NevaMessagingDelegate() override;

  // MessagingDelegate:
  std::optional<base::Value::Dict> MaybeGetTabInfo(
      content::WebContents* web_contents) override;
  content::WebContents* GetWebContentsByTabId(
      content::BrowserContext* browser_context,
      int tab_id) override;
  std::unique_ptr<extensions::MessagePort> CreateReceiverForTab(
      base::WeakPtr<extensions::MessagePort::ChannelDelegate> channel_delegate,
      const std::string& extension_id,
      const extensions::PortId& receiver_port_id,
      content::WebContents* receiver_contents,
      int receiver_frame_id,
      const std::string& receiver_document_id) override;
};

}  // namespace neva

#endif  // NEVA_EXTENSIONS_BROWSER_API_MESSAGING_NEVA_MESSAGING_DELEGATE_H_
