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

#ifndef NEVA_BROWSER_SHELL_SERVICE_BROWSER_SHELL_IPC_ENDPOINT_IMPL_H_
#define NEVA_BROWSER_SHELL_SERVICE_BROWSER_SHELL_IPC_ENDPOINT_IMPL_H_

#include "base/memory/raw_ptr.h"
#include "mojo/public/cpp/bindings/associated_remote.h"
#include "neva/browser_shell/service/browser_shell_ipc.h"
#include "neva/browser_shell/service/public/mojom/browser_shell_ipc_endpoint.mojom.h"

namespace browser_shell {
class ShellIpc;

class ShellIpcEndpointImpl : public mojom::ShellIpcEndpoint,
                             public ShellIpcEndpointDelegate {
 public:
  ShellIpcEndpointImpl(std::string channel, ShellIpc* shell_ipc);
  ShellIpcEndpointImpl(const ShellIpcEndpointImpl&) = delete;
  ShellIpcEndpointImpl& operator=(const ShellIpcEndpointImpl&) = delete;
  ~ShellIpcEndpointImpl() override;

  // ShellIpcEndpointDelegate
  const std::string& GetChannelName() const override;
  void Handle(const std::string& event, const std::string& json) override;

  // mojom::ShellIpcEndpoint
  void BindClient(BindClientCallback callback) override;
  void Post(const std::string& event, const std::string& json) override;

 private:
  const raw_ptr<ShellIpc> shell_ipc_;
  const std::string channel_;
  mojo::AssociatedRemote<mojom::ShellIpcEndpointClient> remote_client_;
};

}  // namespace browser_shell

#endif  // NEVA_BROWSER_SHELL_SERVICE_BROWSER_SHELL_IPC_ENDPOINT_IMPL_H_
