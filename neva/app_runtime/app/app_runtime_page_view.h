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

#ifndef NEVA_APP_RUNTIME_APP_APP_RUNTIME_PAGE_VIEW_H_
#define NEVA_APP_RUNTIME_APP_APP_RUNTIME_PAGE_VIEW_H_

#include <list>
#include <map>
#include <memory>

#include "base/memory/raw_ptr.h"
#include "neva/app_runtime/app/app_runtime_page_view_delegate.h"

namespace views {
class WebView;
class View;
}

namespace neva_app_runtime {

class PageContents;
class ShellWindow;

class PageView {
 public:
  PageView(PageViewDelegate* delegate = nullptr);
  PageView(const PageView&) = delete;
  PageView& operator=(const PageView&) = delete;
  virtual ~PageView();

  void SetDelegate(PageViewDelegate* delegate);
  PageViewDelegate* GetDelegate() const;

  uint64_t GetID() const;
  PageContents* GetPageContents() const;
  std::unique_ptr<PageContents> SetPageContents(
      std::unique_ptr<PageContents> page_contents);
  std::unique_ptr<PageContents> RemovePageContents();

  PageView* AddChildPageView(std::unique_ptr<PageView> page_view);
  std::unique_ptr<PageView> RemoveChildPageView(PageView* page_view);
  std::list<PageView*> GetChildPageViews() const;
  void DeleteAllChildViews();

  ShellWindow* GetParentShellWindow() const;
  PageView* GetParentPageView() const;
  void UpdatePageContents();
  void ResetPageContents();

  void SetBounds(int x, int y, int width, int height);
  void SetVisible(
      bool visible,
      VisibilityChangeReason reason = VisibilityChangeReason::kUnknown);
  bool IsVisible() const;
  void BringToFront();
  void BringToFront(PageView* child_page_view);
  void SendToBack();
  void SendToBack(PageView* child_page_view);

  views::View* GetView() const;
  void CloseIfMain();

 private:
  friend ShellWindow;

  void SetParentShellWindow(ShellWindow* shell_window);
  void SetParentPageView(PageView* page_view);

  const uint64_t id_;

  raw_ptr<PageView> parent_page_view_ = nullptr;
  raw_ptr<ShellWindow> parent_shell_window_ = nullptr;

  raw_ptr<PageViewDelegate> delegate_ = nullptr;
  PageViewDelegate stub_delegate_;
  std::unique_ptr<views::WebView> web_view_;
  std::unique_ptr<PageContents> page_contents_;
  std::map<PageView*, std::unique_ptr<PageView>> child_page_views_;
  std::map<views::View*, PageView*> map_view_to_page_view_;
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_APP_APP_RUNTIME_PAGE_VIEW_H_
