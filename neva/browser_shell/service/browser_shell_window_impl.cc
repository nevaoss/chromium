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

#include "neva/browser_shell/service/browser_shell_window_impl.h"

#include "base/logging.h"
#include "mojo/public/cpp/bindings/self_owned_receiver.h"
#include "neva/app_runtime/app/app_runtime_page_view.h"
#include "neva/app_runtime/app/app_runtime_shell_window.h"
#include "neva/browser_shell/service/browser_shell_page_view_impl.h"
#include "neva/browser_shell/service/browser_shell_service_impl.h"
#include "neva/logging.h"
#include "ui/gfx/geometry/rect.h"

namespace browser_shell {

ShellWindowImpl::ShellWindowImpl(ShellServiceImpl* shell_service,
                                 neva_app_runtime::ShellWindow* shell_window,
                                 std::string name)
    : shell_service_(shell_service),
      name_(std::move(name)),
      shell_window_(shell_window) {
  NEVA_DCHECK(shell_window_);
  shell_window_->SetDelegate(this);
}

ShellWindowImpl::~ShellWindowImpl() = default;

std::string ShellWindowImpl::GetName() const {
  return name_;
}

neva_app_runtime::ShellWindow* ShellWindowImpl::GetShellWindow() const {
  return shell_window_;
}

void ShellWindowImpl::BindPageView(
    mojo::PendingReceiver<mojom::PageView> receiver,
    BindPageViewCallback callback) {
  NOTIMPLEMENTED();
  std::move(callback).Run(0);
}

void ShellWindowImpl::RegisterClient(
    mojo::PendingAssociatedRemote<mojom::ShellWindowClient> remote) {
  remotes_.Add(std::move(remote));
}

void ShellWindowImpl::SyncName(SyncNameCallback callback) {
  std::move(callback).Run(name_);
}

void ShellWindowImpl::OnDisplaySizeChanged(const gfx::Rect& bounds) {
  for (auto& remote : remotes_) {
    remote->OnDisplaySizeChanged(CreateNewRectBounds(bounds));
  }
}

void ShellWindowImpl::OnWindowClosing() {
  NOTIMPLEMENTED();
}

void ShellWindowImpl::VirtuaKeyboardChangeState(bool visible) {
  for (auto& remote : remotes_)
    remote->VirtuaKeyboardChangeState(visible);
}

void ShellWindowImpl::VirtuaKeyboardOverlapTextField(const gfx::Rect& bounds) {
  for (auto& remote : remotes_) {
    remote->VirtuaKeyboardOverlapTextField(CreateNewRectBounds(bounds));
  }
}

mojo::InlinedStructPtr<browser_shell::mojom::Rect>
ShellWindowImpl::CreateNewRectBounds(const gfx::Rect& bounds) {
  return browser_shell::mojom::Rect::New(bounds.x(), bounds.y(), bounds.width(),
                                         bounds.height());
}

}  // namespace browser_shell
