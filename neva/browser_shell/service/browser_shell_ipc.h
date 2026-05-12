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

#ifndef NEVA_BROWSER_SHELL_SERVICE_BROWSER_SHELL_IPC_H_
#define NEVA_BROWSER_SHELL_SERVICE_BROWSER_SHELL_IPC_H_

#include <list>
#include <map>
#include <string>

namespace browser_shell {

struct ShellIpcEndpointDelegate {
  virtual const std::string& GetChannelName() const = 0;
  virtual void Handle(const std::string& event, const std::string& json) = 0;
};

class ShellIpc {
 public:
  ShellIpc();
  ShellIpc(const ShellIpc&) = delete;
  ShellIpc& operator=(const ShellIpc&) = delete;
  ~ShellIpc();

  void Add(ShellIpcEndpointDelegate* endpoint);
  void Remove(ShellIpcEndpointDelegate* endpoint);
  void Notify(const ShellIpcEndpointDelegate* src_endpoint,
              const std::string& event,
              const std::string& json);
 private:
  std::map<std::string, std::list<ShellIpcEndpointDelegate*>> endpoints_;
};

}  // namespace browser_shell

#endif  // NEVA_BROWSER_SHELL_SERVICE_BROWSER_SHELL_IPC_H_
