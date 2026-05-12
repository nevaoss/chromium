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

#ifndef NEVA_BROWSER_SHELL_SERVICE_BROWSER_SHELL_MAIN_PAGE_CONTENTS_IMPL_H_
#define NEVA_BROWSER_SHELL_SERVICE_BROWSER_SHELL_MAIN_PAGE_CONTENTS_IMPL_H_

#include "mojo/public/cpp/bindings/receiver_set.h"
#include "neva/browser_shell/service/browser_shell_page_contents_impl.h"

namespace neva_app_runtime {
class PageContents;
}

namespace browser_shell {

class ShellServiceImpl;

class MainPageContentsImpl : public PageContentsImpl {
 public:
  MainPageContentsImpl(ShellServiceImpl* shell_service,
                   neva_app_runtime::PageContents* page_contents);
  MainPageContentsImpl(const MainPageContentsImpl&) = delete;
  MainPageContentsImpl& operator=(const MainPageContentsImpl&) = delete;
  ~MainPageContentsImpl() override;

  void AddBinding(mojo::PendingReceiver<mojom::PageContents> receiver);

  void LoadProgressChanged(uint32_t progress) override;
  bool RunJSDialog(const std::string& type,
                   const std::string& message) override;
 private:
  mojo::ReceiverSet<mojom::PageContents> receivers_;
};

}  // namespace browser_shell

#endif  // NEVA_BROWSER_SHELL_SERVICE_BROWSER_SHELL_MAIN_PAGE_CONTENTS_IMPL_H_
