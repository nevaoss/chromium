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

#include "neva/browser_shell/service/browser_shell_ipc_endpoint_impl.h"

#include "mojo/public/cpp/bindings/pending_associated_receiver.h"
#include "neva/logging.h"

namespace browser_shell {

ShellIpcEndpointImpl::ShellIpcEndpointImpl(std::string channel,
                                           ShellIpc* shell_ipc)
  : shell_ipc_(shell_ipc), channel_(std::move(channel)) {
  NEVA_DCHECK(shell_ipc_);
  shell_ipc_->Add(this);
}

ShellIpcEndpointImpl::~ShellIpcEndpointImpl() {
  shell_ipc_->Remove(this);
}

const std::string& ShellIpcEndpointImpl::GetChannelName() const {
  return channel_;
}

void ShellIpcEndpointImpl::Handle(const std::string& event,
                                  const std::string& json) {
  remote_client_->Handle(event, json);
}

void ShellIpcEndpointImpl::BindClient(BindClientCallback callback) {
  std::move(callback).Run(remote_client_.BindNewEndpointAndPassReceiver());
}

void ShellIpcEndpointImpl::Post(const std::string& event,
                                const std::string& json) {
  shell_ipc_->Notify(this, event, json);
}

}  // namespace browser_shell
