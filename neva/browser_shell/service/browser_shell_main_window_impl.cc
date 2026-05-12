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

#include "neva/browser_shell/service/browser_shell_main_window_impl.h"

#include "neva/app_runtime/app/app_runtime_shell_window.h"
#include "neva/browser_shell/service/browser_shell_main_page_view_impl.h"
#include "neva/logging.h"

namespace browser_shell {

ShellMainWindowImpl::ShellMainWindowImpl(
    ShellServiceImpl* shell_service,
    neva_app_runtime::ShellWindow* shell_window)
    : ShellWindowImpl(shell_service, shell_window, "shell_main_window") {}

ShellMainWindowImpl::~ShellMainWindowImpl() = default;

void ShellMainWindowImpl::AddBinding(
    mojo::PendingReceiver<mojom::ShellWindow> receiver) {
  receivers_.Add(this, std::move(receiver));
}

void ShellMainWindowImpl::BindPageView(
    mojo::PendingReceiver<mojom::PageView> receiver,
    BindPageViewCallback callback) {
  if (!main_page_view_impl_) {
    auto* shell_window = GetShellWindow();
    NEVA_DCHECK(shell_window);
    auto* page_view = shell_window->GetPageView();
    NEVA_DCHECK(page_view);
    main_page_view_impl_ =
        std::make_unique<MainPageViewImpl>(shell_service_, page_view);
  }

  main_page_view_impl_->AddBinding(std::move(receiver));
  const uint64_t id = main_page_view_impl_->GetID();
  std::move(callback).Run(id);
}

}  // namespace browser_shell
