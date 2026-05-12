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

#include "neva/user_agent/browser/client_hints.h"

#include "base/command_line.h"
#include "neva/user_agent/common/user_agent.h"
#include "neva/user_agent/common/user_agent_switches.h"
#include "third_party/blink/public/common/user_agent/user_agent_metadata.h"

namespace neva_user_agent {

ClientHints::ClientHints()
    : enable_ua_ch_(base::CommandLine::ForCurrentProcess()->HasSwitch(
          kEnableNevaUserAgentClientHints)) {}

network::NetworkQualityTracker* ClientHints::GetNetworkQualityTracker() {
  return nullptr;
}

void ClientHints::GetAllowedClientHintsFromSource(
    const url::Origin& origin,
    blink::EnabledClientHints* client_hints) {
  client_hints->SetIsEnabled(network::mojom::WebClientHintsType::kUA, true);
  client_hints->SetIsEnabled(network::mojom::WebClientHintsType::kUAMobile,
                             true);
  client_hints->SetIsEnabled(network::mojom::WebClientHintsType::kUAPlatform,
                             true);
  client_hints->SetIsEnabled(
      network::mojom::WebClientHintsType::kUAPlatformVersion, true);
  client_hints->SetIsEnabled(network::mojom::WebClientHintsType::kUAArch, true);
  client_hints->SetIsEnabled(network::mojom::WebClientHintsType::kUAModel,
                             true);
  client_hints->SetIsEnabled(network::mojom::WebClientHintsType::kUAFullVersion,
                             true);
}

bool ClientHints::IsJavaScriptAllowed(const GURL& url,
                                      content::RenderFrameHost* parent_rfh) {
  return enable_ua_ch_;
}

blink::UserAgentMetadata ClientHints::GetUserAgentMetadata() {
  return GetDefaultUserAgentMetadata();
}

void ClientHints::PersistClientHints(
    const url::Origin& primary_origin,
    content::RenderFrameHost* parent_rfh,
    const std::vector<network::mojom::WebClientHintsType>& client_hints) {}

void ClientHints::SetAdditionalClientHints(
    const std::vector<network::mojom::WebClientHintsType>&) {}

void ClientHints::ClearAdditionalClientHints() {}

void ClientHints::SetMostRecentMainFrameViewportSize(
    const gfx::Size& viewport_size) {}

gfx::Size ClientHints::GetMostRecentMainFrameViewportSize() {
  return gfx::Size();
}

}  // namespace neva_user_agent
