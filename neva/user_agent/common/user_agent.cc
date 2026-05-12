// Copyright 2017-2018 LG Electronics, Inc.
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

#include "neva/user_agent/common/user_agent.h"

#include "base/command_line.h"
#include "base/logging.h"
#include "components/version_info/version_info.h"
#include "content/public/common/user_agent.h"
#include "net/http/http_util.h"
#include "neva/user_agent/common/user_agent_switches.h"
#include "third_party/blink/public/common/user_agent/user_agent_metadata.h"

namespace neva_user_agent {
namespace {

std::string GetProduct() {
  return std::string(version_info::GetProductNameAndVersionForUserAgent());
}

}  // namespace

std::string GetDefaultUserAgent() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(kUserAgent)) {
    std::string ua = command_line->GetSwitchValueASCII(kUserAgent);
    if (net::HttpUtil::IsValidHeaderValue(ua))
      return ua;
    LOG(WARNING) << "Ignored invalid value for flag --" << kUserAgent;
  }

  std::string product = GetProduct();
  return content::BuildUserAgentFromProduct(product);
}

bool IsUserAgentClientHintsEnabled() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(
      kEnableNevaUserAgentClientHints);
}

blink::UserAgentMetadata GetDefaultUserAgentMetadata() {
  if (!base::CommandLine::ForCurrentProcess()->HasSwitch(
          kEnableNevaUserAgentClientHints)) {
    return blink::UserAgentMetadata();
  }

  blink::UserAgentMetadata metadata;
  metadata.mobile = false;
  metadata.platform = "webOS";
  metadata.platform_version = "";
  metadata.architecture = "";
  metadata.model = "";
  metadata.full_version = version_info::GetVersionNumber();

  blink::UserAgentBrandList brand_version_list;
  brand_version_list.emplace_back("Chromium",
                                  version_info::GetMajorVersionNumber());
  brand_version_list.emplace_back(" Not;A Brand", "99");
  metadata.brand_version_list = brand_version_list;

  return metadata;
}

}  // namespace neva_user_agent
