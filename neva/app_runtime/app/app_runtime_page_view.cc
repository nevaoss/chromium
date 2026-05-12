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

#include "neva/app_runtime/app/app_runtime_page_view.h"

#include "base/logging.h"
#include "neva/app_runtime/app/app_runtime_page_contents.h"
#include "neva/app_runtime/app/app_runtime_page_view_delegate.h"
#include "neva/app_runtime/app/app_runtime_shell_environment.h"
#include "neva/logging.h"
#include "ui/views/controls/webview/webview.h"

namespace neva_app_runtime {

PageView::PageView(PageViewDelegate* delegate)
    : id_(ShellEnvironment::GetInstance()->GetNextIDFor(this)),
      delegate_(delegate ? delegate : &stub_delegate_),
      web_view_(new views::WebView()) {
  web_view_->set_owned_by_client();
}

PageView::~PageView() {
  if (delegate_)
    delegate_->OnDestroying(this);

  ResetPageContents();

  if (page_contents_) {
    page_contents_->SetParentPageView(nullptr);
    ShellEnvironment::GetInstance()->SaveDetachedContents(
        std::move(page_contents_));
  }

  DeleteAllChildViews();
  std::ignore = ShellEnvironment::GetInstance()->Release(this).release();
}

void PageView::SetDelegate(PageViewDelegate* delegate) {
  delegate_ = delegate ? delegate : &stub_delegate_;
}

PageViewDelegate* PageView::GetDelegate() const {
  return (delegate_ == &stub_delegate_) ? nullptr : delegate_;
}

uint64_t PageView::GetID() const {
  return id_;
}

PageContents* PageView::GetPageContents() const {
  return page_contents_.get();
}

std::unique_ptr<PageContents> PageView::SetPageContents(
    std::unique_ptr<PageContents> page_contents) {
  if (page_contents && page_contents->GetParentPageView()) {
    LOG(WARNING) << "Provided PageContents is already set to PageView.";
    return std::unique_ptr<PageContents>();
  }

  auto previous_page_contents = std::move(page_contents_);
  page_contents_ = std::move(page_contents);

  if (page_contents_) {
    page_contents_->SetParentPageView(this);
    if (page_contents_->GetWebContents()) {
      web_view_->SetBrowserContext(
          page_contents_->GetWebContents()->GetBrowserContext());
      web_view_->SetWebContents(page_contents_->GetWebContents());
    }
  } else {
    web_view_->SetBrowserContext(nullptr);
    web_view_->SetWebContents(nullptr);
  }

  if (previous_page_contents) {
    previous_page_contents->SetParentPageView(nullptr);
  }
  return previous_page_contents;
}

std::unique_ptr<PageContents> PageView::RemovePageContents() {
  web_view_->SetWebContents(nullptr);
  return std::move(page_contents_);
}

PageView* PageView::AddChildPageView(std::unique_ptr<PageView> page_view) {
  auto inserted = child_page_views_.insert(
      std::make_pair(page_view.get(), std::move(page_view)));
  if (inserted.second) {
    map_view_to_page_view_[inserted.first->first->GetView()] =
        inserted.first->first;
    web_view_->AddChildView(inserted.first->first->GetView());
    inserted.first->first->SetParentPageView(this);
    return inserted.first->first;
  }
  return nullptr;
}

std::unique_ptr<PageView> PageView::RemoveChildPageView(PageView* page_view) {
  auto it = child_page_views_.find(page_view);
  if (it == child_page_views_.end())
    return std::unique_ptr<PageView>();

  auto detached = std::move(it->second);
  child_page_views_.erase(it);

  detached->SetParentPageView(nullptr);
  web_view_->RemoveChildView(page_view->GetView());
  map_view_to_page_view_.erase(page_view->GetView());
  return detached;
}

std::list<PageView*> PageView::GetChildPageViews() const {
  std::list<PageView*> views;
  for (const auto& child : child_page_views_)
    views.push_back(child.first);
  return views;
}

void PageView::DeleteAllChildViews() {
  for (const auto& child : map_view_to_page_view_)
    web_view_->RemoveChildView(child.first);
  map_view_to_page_view_.clear();
  child_page_views_.clear();
}

ShellWindow* PageView::GetParentShellWindow() const {
  return parent_shell_window_;
}

PageView* PageView::GetParentPageView() const {
  return parent_page_view_;
}

void PageView::UpdatePageContents() {
  if (!page_contents_.get())
    return;

  if (!page_contents_->GetWebContents())
    return;

  web_view_->SetBrowserContext(
      page_contents_->GetWebContents()->GetBrowserContext());
  web_view_->SetWebContents(page_contents_->GetWebContents());
}

void PageView::ResetPageContents() {
  web_view_->SetBrowserContext(nullptr);
  web_view_->SetWebContents(nullptr);
}

void PageView::SetBounds(int x, int y, int width, int height) {
  if (parent_shell_window_) {
    LOG(INFO) << "This PageView is main PageView in ShellWindow, "
              << "its position is managed by layout manager and "
              << "new bounds might be ignored.";
  }
  web_view_->SetBounds(x, y, width, height);
}

void PageView::SetVisible(bool visible, VisibilityChangeReason reason) {
  if (web_view_->GetVisible() != visible) {
    web_view_->SetVisible(visible);
    delegate_->OnVisibilityChanged(visible, reason);
  }
}

bool PageView::IsVisible() const {
  return web_view_->GetVisible();
}

void PageView::BringToFront() {
  if (parent_page_view_)
    parent_page_view_->GetView()->ReorderChildView(web_view_.get(), -1);
}

void PageView::BringToFront(PageView* child_page_view) {
  NEVA_DCHECK(child_page_view != nullptr);

  if (child_page_views_.count(child_page_view) > 0) {
    auto* child_view = child_page_view->GetView();
    web_view_->ReorderChildView(child_view, -1);
  }
}

void PageView::SendToBack() {
  if (parent_page_view_)
    parent_page_view_->GetView()->ReorderChildView(web_view_.get(), 1);
}

void PageView::SendToBack(PageView* child_page_view) {
  NEVA_DCHECK(child_page_view != nullptr);

  if (child_page_views_.count(child_page_view) > 0) {
    auto* child_view = child_page_view->GetView();
    web_view_->ReorderChildView(child_view, 1);
  }
}

views::View* PageView::GetView() const {
  return web_view_.get();
}

void PageView::CloseIfMain() {
  if (parent_shell_window_) {
    if (page_contents_.get()) {
      ShellEnvironment::GetInstance()->Remove(page_contents_.get());
    }
    DeleteAllChildViews();
  }
}

void PageView::SetParentShellWindow(ShellWindow* shell_window) {
  NEVA_DCHECK(!shell_window || !parent_page_view_);
  parent_shell_window_ = shell_window;
}

void PageView::SetParentPageView(PageView* page_view) {
  NEVA_DCHECK(!page_view || !parent_shell_window_);
  parent_page_view_ = page_view;
}

}  // namespace neva_app_runtime
