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

#ifndef NEVA_BROWSER_SHELL_SERVICE_BROWSER_SHELL_PAGE_VIEW_IMPL_H_
#define NEVA_BROWSER_SHELL_SERVICE_BROWSER_SHELL_PAGE_VIEW_IMPL_H_

#include "base/memory/raw_ptr.h"
#include "mojo/public/cpp/bindings/associated_remote.h"
#include "mojo/public/cpp/bindings/pending_associated_remote.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/remote_set.h"
#include "neva/app_runtime/app/app_runtime_page_view_delegate.h"
#include "neva/browser_shell/service/public/mojom/browser_shell_page_view.mojom.h"

namespace neva_app_runtime {
class PageView;
}

namespace browser_shell {

class ShellServiceImpl;

class PageViewImpl : public mojom::PageView,
                     public neva_app_runtime::PageViewDelegate {
 public:
  PageViewImpl(ShellServiceImpl* shell_service,
               neva_app_runtime::PageView* page_view);
  PageViewImpl(const PageViewImpl&) = delete;
  PageViewImpl& operator=(const PageViewImpl&) = delete;
  ~PageViewImpl() override;

  uint64_t GetID() const;
  neva_app_runtime::PageView* GetPageView() const;

  // mojom::PageView
  void BindPageContents(mojo::PendingReceiver<mojom::PageContents> receiver,
                        BindPageContentsCallback callback) override;
  void RegisterClient(
      mojo::PendingAssociatedRemote<mojom::PageViewClient> remote) override;

  void SyncId(SyncIdCallback callback) override;

  void SetBounds(int32_t x, int32_t y, int32_t w, int32_t h) override;
  void SetVisible(bool visible) override;
  void BringToFront() override;
  void BringToFrontByID(uint64_t id) override;
  void SendToBack() override;
  void SendToBackByID(uint64_t id) override;
  void AddChildView(uint64_t id) override;
  void RemoveChildView(uint64_t id) override;
  void SetPageContents(uint64_t id) override;

  // neva_app_runtime::PageViewDelegate
  void OnDestroying(neva_app_runtime::PageView* view) override;
  void OnVisibilityChanged(
      bool visible,
      neva_app_runtime::VisibilityChangeReason reason) override;

 protected:
  const raw_ptr<ShellServiceImpl> shell_service_;

 private:
  const raw_ptr<neva_app_runtime::PageView> page_view_;
  mojo::AssociatedRemoteSet<mojom::PageViewClient> remotes_;
};

}  // namespace browser_shell

#endif  // NEVA_BROWSER_SHELL_SERVICE_BROWSER_SHELL_PAGE_VIEW_IMPL_H_
