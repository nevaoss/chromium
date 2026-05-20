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

#include "neva/browser_shell/service/browser_shell_main_page_contents_impl.h"

#include "base/logging.h"

namespace browser_shell {

MainPageContentsImpl::MainPageContentsImpl(
    ShellServiceImpl* shell_service,
    neva_app_runtime::PageContents* page_contents)
    : PageContentsImpl(shell_service, page_contents) {}

MainPageContentsImpl::~MainPageContentsImpl() = default;

void MainPageContentsImpl::AddBinding(
    mojo::PendingReceiver<mojom::PageContents> receiver) {
  receivers_.Add(this, std::move(receiver));
}

void MainPageContentsImpl::LoadProgressChanged(uint32_t progress) {
  LOG(INFO) << "Load progress is not supported for main page of BrowserShell";
}

bool MainPageContentsImpl::RunJSDialog(const std::string& type,
                                       const std::string& message) {
  // That means that the JS Dialog request is done by main application page.
  // alert, confirm and prompt are modal, so the JS in main page is now frozen
  // and we cannot ask it to process any event.
  return false;
}

}  // namespace browser_shell
