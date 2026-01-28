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

#include "neva/browser_shell/service/browser_shell_main_page_view_impl.h"

#include "neva/app_runtime/app/app_runtime_page_view.h"
#include "neva/browser_shell/service/browser_shell_main_page_contents_impl.h"
#include "neva/logging.h"

namespace browser_shell {

MainPageViewImpl::MainPageViewImpl(ShellServiceImpl* shell_service,
                           neva_app_runtime::PageView* page_view)
    : PageViewImpl(shell_service, page_view) {}

MainPageViewImpl::~MainPageViewImpl() = default;

void MainPageViewImpl::AddBinding(
    mojo::PendingReceiver<mojom::PageView> receiver) {
  receivers_.Add(this, std::move(receiver));
}

void MainPageViewImpl::BindPageContents(
    mojo::PendingReceiver<mojom::PageContents> receiver,
    BindPageContentsCallback callback) {

  if (!main_page_contents_impl_) {
    auto* page_view = GetPageView();
    NEVA_DCHECK(page_view);
    auto* page_contents = page_view->GetPageContents();
    NEVA_DCHECK(page_contents != nullptr);
    main_page_contents_impl_ =
        std::make_unique<MainPageContentsImpl>(shell_service_, page_contents);
  }

  const uint64_t id = main_page_contents_impl_->GetID();
  main_page_contents_impl_->AddBinding(std::move(receiver));

  auto info = browser_shell::mojom::PageContentsCreationInfo::New(
      main_page_contents_impl_->GetActiveState(),
      main_page_contents_impl_->GetErrorPageHiding(),
      main_page_contents_impl_->GetUserAgent(),
      main_page_contents_impl_->GetZoomFactor());

  std::move(callback).Run(id, std::move(info));
}

}  // namespace browser_shell
