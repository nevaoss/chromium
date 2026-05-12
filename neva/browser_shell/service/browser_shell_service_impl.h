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

#ifndef NEVA_BROWSER_SHELL_SERVICE_BROWSER_SHELL_SERVICE_IMPL_H_
#define NEVA_BROWSER_SHELL_SERVICE_BROWSER_SHELL_SERVICE_IMPL_H_

#include "base/component_export.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "mojo/public/cpp/bindings/receiver_set.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "mojo/public/cpp/bindings/remote_set.h"
#include "mojo/public/cpp/bindings/unique_receiver_set.h"
#include "neva/app_runtime/app/app_runtime_shell_observer.h"
#include "neva/browser_shell/service/browser_shell_ipc.h"
#include "neva/browser_shell/service/public/mojom/browser_shell_cookie_manager.mojom.h"
#include "neva/browser_shell/service/public/mojom/browser_shell_ipc_endpoint.mojom.h"
#include "neva/browser_shell/service/public/mojom/browser_shell_permissions.mojom.h"
#include "neva/browser_shell/service/public/mojom/browser_shell_service.mojom.h"
#include "neva/browser_shell/service/public/mojom/browser_shell_webrequest.mojom.h"

namespace neva_app_runtime {
class Shell;
}

namespace browser_shell {

class PageContentsImpl;
class PageViewImpl;
class PermissionsImpl;
class ShellMainWindowImpl;

class COMPONENT_EXPORT(BROWSER_SHELL_SERVICE) ShellServiceImpl :
    public mojom::ShellService,
    public neva_app_runtime::ShellObserver {
 public:
  explicit ShellServiceImpl(std::unique_ptr<neva_app_runtime::Shell> shell);
  ShellServiceImpl(const ShellServiceImpl&) = delete;
  ShellServiceImpl& operator=(const ShellServiceImpl&) = delete;
  ~ShellServiceImpl() override;

  void AddBinding(mojo::PendingReceiver<mojom::ShellService> receiver) override;
  void RegisterClient(
      mojo::PendingRemote<mojom::ShellServiceClient> remote) override;

  void BindShellWindow(mojo::PendingReceiver<mojom::ShellWindow> receiver,
                       BindShellWindowCallback callback) override;

  void BindPermissions(
      mojo::PendingReceiver<mojom::Permissions> receiver) override;

  void CreateCookieManager(mojo::PendingReceiver<mojom::CookieManager> receiver,
                           const std::string& partition) override;

  void CreatePageView(mojo::PendingReceiver<mojom::PageView> receiver,
                      const std::string& json,
                      CreatePageViewCallback callback) override;

  void CreatePageContents(mojo::PendingReceiver<mojom::PageContents> receiver,
                          const std::string& json,
                          CreatePageContentsCallback callback) override;

  void CreateShellIpcEndpoint(
      mojo::PendingReceiver<mojom::ShellIpcEndpoint> receiver,
      const std::string& name) override;

  void TouchSession(const std::string& partition) override;

  void CreateWebRequest(mojo::PendingReceiver<mojom::WebRequest> receiver,
                        const std::string& partition) override;

  void AddUniqueReceiver(std::unique_ptr<PageContentsImpl> page_contents_impl,
                         mojo::PendingReceiver<mojom::PageContents> receiver);

  void AddUniqueReceiver(std::unique_ptr<PageViewImpl> page_view_impl,
                         mojo::PendingReceiver<mojom::PageView> receiver);

  // neva_app_runtime::ShellObserver
  void OnMainWindowClosing() override;

 private:
  void InitializePermissions();

  ShellIpc shell_ipc_;
  std::unique_ptr<neva_app_runtime::Shell> shell_;
  mojo::RemoteSet<mojom::ShellServiceClient> remotes_;
  mojo::ReceiverSet<mojom::ShellService> receivers_;

  std::unique_ptr<ShellMainWindowImpl> main_shell_window_impl_;
  std::unique_ptr<PermissionsImpl> permissions_impl_;

  mojo::UniqueReceiverSet<mojom::CookieManager> cookie_manager_receivers_;
  mojo::UniqueReceiverSet<mojom::WebRequest> webrequest_receivers_;
  mojo::UniqueReceiverSet<mojom::PageContents> page_contents_receivers_;
  mojo::UniqueReceiverSet<mojom::PageView> page_view_receivers_;
  mojo::UniqueReceiverSet<mojom::ShellIpcEndpoint> ipc_endpoint_receivers_;
};

}  // namespace browser_shell

#endif  // NEVA_BROWSER_SHELL_SERVICE_BROWSER_SHELL_SERVICE_IMPL_H_
