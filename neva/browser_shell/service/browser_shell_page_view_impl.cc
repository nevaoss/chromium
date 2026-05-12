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

#include "neva/browser_shell/service/browser_shell_page_view_impl.h"

#include "base/logging.h"
#include "neva/app_runtime/app/app_runtime_page_contents.h"
#include "neva/app_runtime/app/app_runtime_page_view.h"
#include "neva/app_runtime/app/app_runtime_shell_environment.h"
#include "neva/browser_shell/service/browser_shell_service_impl.h"
#include "neva/browser_shell/service/browser_shell_page_contents_impl.h"
#include "neva/logging.h"

namespace browser_shell {

PageViewImpl::PageViewImpl(ShellServiceImpl* shell_service,
                           neva_app_runtime::PageView* page_view)
    : shell_service_(shell_service),
      page_view_(page_view) {
  NEVA_DCHECK(page_view_);
  page_view_->SetDelegate(this);
}

PageViewImpl::~PageViewImpl() {
  page_view_->SetDelegate(nullptr);
  neva_app_runtime::ShellEnvironment::GetInstance()->Remove(page_view_);
}

uint64_t PageViewImpl::GetID() const {
  return page_view_->GetID();
}

neva_app_runtime::PageView* PageViewImpl::GetPageView() const {
  return page_view_;
}

void PageViewImpl::BindPageContents(
    mojo::PendingReceiver<mojom::PageContents> receiver,
    BindPageContentsCallback callback) {
  auto* page_contents = page_view_->GetPageContents();
  NEVA_DCHECK(page_contents != nullptr);
  auto page_contents_impl =
    std::make_unique<PageContentsImpl>(shell_service_, page_contents);
  const uint64_t id = page_contents_impl->GetID();
  auto info = browser_shell::mojom::PageContentsCreationInfo::New(
      page_contents_impl->GetActiveState(),
      page_contents_impl->GetErrorPageHiding(),
      page_contents_impl->GetUserAgent(),
      page_contents_impl->GetZoomFactor());

  shell_service_->AddUniqueReceiver(std::move(page_contents_impl),
                                    std::move(receiver));
  std::move(callback).Run(id, std::move(info));
}

void PageViewImpl::RegisterClient(
    mojo::PendingAssociatedRemote<mojom::PageViewClient> remote) {
  remotes_.Add(std::move(remote));
}

void PageViewImpl::SyncId(SyncIdCallback callback) {
  std::move(callback).Run(GetID());
}

void PageViewImpl::SetBounds(int32_t x, int32_t y, int32_t w, int32_t h) {
  page_view_->SetBounds(x, y, w, h);
}

void PageViewImpl::SetVisible(bool visible) {
  page_view_->SetVisible(visible);
}

void PageViewImpl::BringToFront() {
  page_view_->BringToFront();
}

void PageViewImpl::BringToFrontByID(uint64_t id) {
  auto* child_page_view =
      neva_app_runtime::ShellEnvironment::GetInstance()->GetViewPtr(id);
  if (child_page_view)
    page_view_->BringToFront(child_page_view);
}

void PageViewImpl::SendToBack() {
  page_view_->SendToBack();
}

void PageViewImpl::SendToBackByID(uint64_t id) {
  auto* child_page_view =
      neva_app_runtime::ShellEnvironment::GetInstance()->GetViewPtr(id);
  if (child_page_view)
    page_view_->SendToBack(child_page_view);
}

void PageViewImpl::AddChildView(uint64_t id) {
  auto* child_ptr =
      neva_app_runtime::ShellEnvironment::GetInstance()->GetViewPtr(id);
  if (!child_ptr)
    return;

  auto child =
      neva_app_runtime::ShellEnvironment::GetInstance()->ReleaseDetachedView(
          child_ptr);
  if (child)
    page_view_->AddChildPageView(std::move(child));
}

void PageViewImpl::RemoveChildView(uint64_t id) {
  auto* child_ptr =
      neva_app_runtime::ShellEnvironment::GetInstance()->GetViewPtr(id);
  if (!child_ptr)
    return;

  auto child = page_view_->RemoveChildPageView(child_ptr);
  if (child.get() && child->GetDelegate()) {
    neva_app_runtime::ShellEnvironment::GetInstance()->SaveDetachedView(
        std::move(child));
  }
}

void PageViewImpl::SetPageContents(uint64_t id) {
  auto* contents_ptr =
      neva_app_runtime::ShellEnvironment::GetInstance()->GetContentsPtr(id);
  if (!contents_ptr)
    return;

  auto contents = neva_app_runtime::ShellEnvironment::GetInstance()
                      ->ReleaseDetachedContents(contents_ptr);
  if (!contents.get()) {
    LOG(WARNING) << "The PageContents is already set to another PageView.";
    return;
  }

  auto previos_contents = page_view_->SetPageContents(std::move(contents));
  if (previos_contents.get() && previos_contents->GetDelegate()) {
    neva_app_runtime::ShellEnvironment::GetInstance()->SaveDetachedContents(
        std::move(previos_contents));
  }
}

void PageViewImpl::OnDestroying(neva_app_runtime::PageView* view) {}

void PageViewImpl::OnVisibilityChanged(
    bool visible,
    neva_app_runtime::VisibilityChangeReason reason) {
  for (auto& remote : remotes_)
    remote->VisibilityChanged(visible, static_cast<unsigned int>(reason));
}

}  // namespace browser_shell
