// Copyright 2021 LG Electronics, Inc.
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

#ifndef NEVA_USER_AGENT_BROWSER_CLIENT_HINTS_H_
#define NEVA_USER_AGENT_BROWSER_CLIENT_HINTS_H_

#include "content/public/browser/client_hints_controller_delegate.h"

namespace blink {
struct UserAgentMetadata;
}  // namespace blink

namespace neva_user_agent {

class ClientHints : public content::ClientHintsControllerDelegate {
 public:
  ClientHints();
  ClientHints(const ClientHints&) = delete;
  ClientHints& operator=(const ClientHints&) = delete;
  ~ClientHints() override = default;

  network::NetworkQualityTracker* GetNetworkQualityTracker() override;

  void GetAllowedClientHintsFromSource(
      const url::Origin& origin,
      blink::EnabledClientHints* client_hints) override;

  bool IsJavaScriptAllowed(const GURL& url,
                           content::RenderFrameHost* parent_rfh) override;

  blink::UserAgentMetadata GetUserAgentMetadata() override;

  void PersistClientHints(const url::Origin& primary_origin,
                          content::RenderFrameHost* parent_rfh,
                          const std::vector<network::mojom::WebClientHintsType>&
                              client_hints) override;

  void SetAdditionalClientHints(
      const std::vector<network::mojom::WebClientHintsType>&) override;

  void ClearAdditionalClientHints() override;

  void SetMostRecentMainFrameViewportSize(
      const gfx::Size& viewport_size) override;

  gfx::Size GetMostRecentMainFrameViewportSize() override;

 private:
  bool enable_ua_ch_;
};

}  // namespace neva_user_agent

#endif  // NEVA_USER_AGENT_BROWSER_CLIENT_HINTS_H_
