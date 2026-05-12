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

#include "neva/browser_shell/service/browser_shell_ipc.h"

#include "neva/logging.h"

namespace browser_shell {

ShellIpc::ShellIpc() = default;
ShellIpc::~ShellIpc() = default;

void ShellIpc::Add(ShellIpcEndpointDelegate* endpoint) {
  if (endpoint) {
    Remove(endpoint);
    endpoints_[endpoint->GetChannelName()].push_back(endpoint);
  }
}

void ShellIpc::Remove(ShellIpcEndpointDelegate* endpoint) {
  auto it = endpoints_.find(endpoint->GetChannelName());
  if (it != endpoints_.end())
    it->second.remove(endpoint);
}

void ShellIpc::Notify(const ShellIpcEndpointDelegate* src_endpoint,
                      const std::string& event,
                      const std::string& json) {
  NEVA_DCHECK(src_endpoint);
  auto it = endpoints_.find(src_endpoint->GetChannelName());
  if (it == endpoints_.end())
    return;

  for (auto* endpoint : it->second) {
    if (endpoint != src_endpoint)
      endpoint->Handle(event, json);
  }
}

}  // namespace browser_shell
