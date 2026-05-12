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

#include "neva/browser_shell/service/browser_shell_page_contents_impl.h"

#include "base/logging.h"
#include "mojo/public/cpp/bindings/self_owned_receiver.h"
#include "neva/app_runtime/app/app_runtime_page_contents.h"
#include "neva/app_runtime/app/app_runtime_page_view.h"
#include "neva/app_runtime/app/app_runtime_shell.h"
#include "neva/app_runtime/app/app_runtime_shell_environment.h"
#include "neva/app_runtime/app/app_runtime_visible_region_capture.h"
#include "neva/app_runtime/public/app_runtime_constants.h"
#include "neva/browser_shell/service/browser_shell_service_impl.h"
#include "neva/logging.h"

namespace browser_shell {

PageContentsImpl::PageContentsImpl(
    ShellServiceImpl* shell_service,
    neva_app_runtime::PageContents* page_contents)
    : shell_service_(shell_service),
      page_contents_(page_contents) {
  NEVA_DCHECK(page_contents_);
  if (page_contents_)
    page_contents_->SetDelegate(this);
}

PageContentsImpl::PageContentsImpl(ShellServiceImpl* shell_service, uint64_t id)
    : shell_service_(shell_service),
      page_contents_(
          neva_app_runtime::ShellEnvironment::GetInstance()->GetContentsPtr(
              id)) {
  NEVA_DCHECK(page_contents_);
  if (page_contents_) {
    NEVA_DCHECK(!page_contents_->GetDelegate());
    page_contents_->SetDelegate(this);
  }
}

PageContentsImpl::~PageContentsImpl() {
  if (page_contents_) {
    page_contents_->SetDelegate(nullptr);

    auto* parent_page_view = page_contents_->GetParentPageView();
    if (parent_page_view) {
      parent_page_view->SetPageContents(
          std::unique_ptr<neva_app_runtime::PageContents>());
    }

    neva_app_runtime::ShellEnvironment::GetInstance()->Remove(page_contents_);
  }
}

bool PageContentsImpl::GetActiveState() {
  return page_contents_->IsActive();
}

uint64_t PageContentsImpl::GetID() const {
  return page_contents_->GetID();
}

std::string PageContentsImpl::GetUserAgent() const {
  return page_contents_->GetUserAgent();
}

double PageContentsImpl::GetZoomFactor() const {
  return page_contents_->GetZoomFactor();
}

neva_app_runtime::PageContents* PageContentsImpl::GetPageContents() const {
  return page_contents_;
}

void PageContentsImpl::RegisterClient(
    mojo::PendingAssociatedRemote<mojom::PageContentsClient> remote) {
  remotes_.Add(std::move(remote));
}

void PageContentsImpl::Activate() {
  page_contents_->Activate();
}

void PageContentsImpl::AckAuthChallenge(const std::string& login,
                                        const std::string& passwd,
                                        const std::string& url) {
  page_contents_->AckAuthChallenge(login, passwd, url);
}

void PageContentsImpl::AckPermission(bool ack, uint64_t id) {
  page_contents_->AckPermission(ack, id);
}

void PageContentsImpl::CaptureVisibleRegion(
    const std::string& format,
    int32_t quality,
    int32_t output_width,
    int32_t output_height,
    CaptureVisibleRegionCallback callback) {
  // Check if another capture for this PageContent is in progress.
  if (capture_visible_region_callback_.get() != nullptr) {
    std::move(callback).Run(std::string());
    return;
  }

  capture_visible_region_callback_ =
      std::make_unique<CaptureVisibleRegionCallback>(std::move(callback));
  page_contents_->CaptureVisibleRegion(
      format, quality, output_width, output_height);
}

void PageContentsImpl::BindNewPageContentsById(
    mojo::PendingReceiver<mojom::PageContents> receiver,
    uint64_t id,
    BindNewPageContentsByIdCallback callback) {
  auto page_contents_impl =
      std::make_unique<PageContentsImpl>(shell_service_, id);
  shell_service_->AddUniqueReceiver(std::move(page_contents_impl),
                                    std::move(receiver));
  auto info = browser_shell::mojom::PageContentsCreationInfo::New(
      GetActiveState(), GetErrorPageHiding(), GetUserAgent(), GetZoomFactor());
  std::move(callback).Run(id, std::move(info));
}

void PageContentsImpl::ClearData(const std::string& clear_options,
                                 const std::string& clear_data_type_set) {
  page_contents_->ClearData(clear_options, clear_data_type_set);
}

void PageContentsImpl::Deactivate() {
  page_contents_->Deactivate();
}

void PageContentsImpl::SyncId(SyncIdCallback callback) {
  std::move(callback).Run(GetID());
}

void PageContentsImpl::CloseJSDialog(bool success,
                                     const std::string& response) {
  page_contents_->CloseJSDialog(success, response);
}

void PageContentsImpl::ExecuteJavaScriptInAllFrames(const std::string& code) {
  page_contents_->ExecuteJavaScriptInAllFrames(code);
}

void PageContentsImpl::ExecuteJavaScriptInMainFrame(const std::string& code) {
  page_contents_->ExecuteJavaScriptInMainFrame(code);
}

void PageContentsImpl::ExitFullscreen() {
  page_contents_->ExitFullscreen();
}

void PageContentsImpl::SyncActiveState(SyncActiveStateCallback callback) {
  bool state = page_contents_->IsActive();
  std::move(callback).Run(state);
}

void PageContentsImpl::GoBack() {
  page_contents_->GoBack();
}

void PageContentsImpl::GoForward() {
  page_contents_->GoForward();
}

void PageContentsImpl::LoadURL(const std::string& url,
                               LoadURLCallback callback) {
  page_contents_->LoadURL(url);
  std::move(callback).Run(url);
}

void PageContentsImpl::Reload() {
  page_contents_->Reload();
}

void PageContentsImpl::ResumeDOM() {
  page_contents_->ResumeDOM();
}

void PageContentsImpl::ResumeMedia() {
  page_contents_->ResumeMedia();
}

void PageContentsImpl::ScrollByY(int y_shift) {
  page_contents_->ScrollByY(y_shift);
}

void PageContentsImpl::SetAcceptedLanguages(const std::string& languages) {
  page_contents_->SetAcceptedLanguages(languages);
}

void PageContentsImpl::SetErrorPageHiding(bool enable) {
  page_contents_->SetErrorPageHiding(enable);
}

void PageContentsImpl::SetFocus() {
  page_contents_->SetFocus();
}

void PageContentsImpl::SetKeyCodesFilter(
    const std::vector<std::string>& key_codes) {
  page_contents_->SetKeyCodesFilter(key_codes);
}

void PageContentsImpl::SetPageBaseBackgroundColor(const std::string& color) {
  page_contents_->SetPageBaseBackgroundColor(color);
}

void PageContentsImpl::SetSuppressSSLErrorPolicy(bool suppress_errors) {
  neva_app_runtime::SSLCertErrorPolicy policy =
      suppress_errors ? neva_app_runtime::SSL_CERT_ERROR_POLICY_IGNORE
                      : neva_app_runtime::SSL_CERT_ERROR_POLICY_DEFAULT;
  page_contents_->SetSSLCertErrorPolicy(policy);
}

void PageContentsImpl::SetZoomFactor(double factor) {
  page_contents_->SetZoomFactor(factor);
}

void PageContentsImpl::Stop() {
  page_contents_->Stop();
}

void PageContentsImpl::SuspendDOM() {
  page_contents_->SuspendDOM();
}

void PageContentsImpl::SuspendMedia() {
  page_contents_->SuspendMedia();
}

void PageContentsImpl::ActivateContents() {
  for (auto& remote : remotes_)
    remote->ActivateContents();
}

void PageContentsImpl::EnterHtmlFullscreen() {
  for (auto& remote : remotes_)
    remote->EnterHtmlFullscreen();
}

void PageContentsImpl::DidFailLoad(const std::string& url,
                                   bool is_main_frame,
                                   const std::string& error,
                                   int error_code) {
  for (auto& remote : remotes_)
    remote->DidFailLoad(url, is_main_frame, error, error_code);
}

void PageContentsImpl::DidFinishLoad(const std::string& url,
                                     bool is_main_frame) {
  for (auto& remote : remotes_)
    remote->DidFinishLoad(url, is_main_frame);
}

void PageContentsImpl::DidFinishNavigation(const std::string& url) {
  for (auto& remote : remotes_)
    remote->DidFinishNavigation(url);
}

void PageContentsImpl::DidPushHistoryNavigation(const std::string& url) {
  for (auto& remote : remotes_)
    remote->DidPushHistoryNavigation(url);
}

void PageContentsImpl::DidStartLoading() {
  for (auto& remote : remotes_)
    remote->DidStartLoading();
}

void PageContentsImpl::DidStartNavigation(const std::string& url) {
  for (auto& remote : remotes_)
    remote->DidStartNavigation(url);
}

void PageContentsImpl::DidStopLoading() {
  for (auto& remote : remotes_)
    remote->DidStopLoading();
}

void PageContentsImpl::DidUpdateFaviconUrl(
    const std::vector<neva_app_runtime::FaviconInfo>& info) {
  for (auto& remote : remotes_) {
    std::vector<browser_shell::mojom::FaviconInfoPtr> sending_info;
    sending_info.reserve(info.size());
    for (const auto& favicon : info) {
      auto favicon_mojom = browser_shell::mojom::FaviconInfo::New();

      favicon_mojom->url = favicon.url;
      favicon_mojom->type = favicon.type;

      favicon_mojom->sizes.reserve(favicon.sizes.size());
      for (const auto& icon_size : favicon.sizes) {
        auto size_mojom = browser_shell::mojom::FaviconSize::New();
        size_mojom->width = icon_size.width;
        size_mojom->height = icon_size.height;

        favicon_mojom->sizes.push_back(std::move(size_mojom));
      }
      sending_info.push_back(std::move(favicon_mojom));
    }
    remote->DidUpdateFaviconUrl(std::move(sending_info));
  }
}

void PageContentsImpl::DOMReady() {
  for (auto& remote : remotes_)
    remote->DOMReady();
}

void PageContentsImpl::LeaveHtmlFullscreen() {
  for (auto& remote : remotes_)
    remote->LeaveHtmlFullscreen();
}

void PageContentsImpl::LoadProgressChanged(uint32_t progress) {
  auto* parent_page_view = page_contents_->GetParentPageView();
  if (parent_page_view && parent_page_view->GetParentShellWindow()) {
    LOG(INFO) << "Load progress is not supported for main page of BrowserShell";
    return;
  }

  for (auto& remote : remotes_)
    remote->LoadProgressChanged(progress);
}

void PageContentsImpl::NavigationEntryCommitted() {
  for (auto& remote : remotes_) {
    auto state = mojom::PageContentsState::New();
    state->can_go_back = page_contents_->CanGoBack();
    state->can_go_forward = page_contents_->CanGoForward();
    remote->OnNavigationEntryCommitted(std::move(state));
  }
}

void PageContentsImpl::OnAcceptedLanguagesChanged(
    const std::string& languages) {
  for (auto& remote : remotes_)
    remote->OnAcceptedLanguagesChanged(languages);
}

void PageContentsImpl::OnAuthChallenge(
    neva_app_runtime::AuthChallengeInfo& challenge) {
  for (auto& remote : remotes_) {
    auto mojom_challenge_info = browser_shell::mojom::AuthChallenge::New();

    mojom_challenge_info->is_proxy = challenge.is_proxy;
    mojom_challenge_info->port = challenge.port;
    mojom_challenge_info->url = challenge.url;
    mojom_challenge_info->scheme = challenge.scheme;
    mojom_challenge_info->host = challenge.host;
    mojom_challenge_info->realm = challenge.realm;

    remote->OnAuthChallenge(std::move(mojom_challenge_info));
  }
}

void PageContentsImpl::OnCapturingMediaStreamChanged(
    bool is_capturing_media_stream) {
  for (auto& remote : remotes_) {
    remote->OnCapturingMediaStreamChanged(is_capturing_media_stream);
  }
}

void PageContentsImpl::OnClose() {
  auto* parent_page_view = page_contents_->GetParentPageView();
  if (parent_page_view && parent_page_view->GetParentShellWindow()) {
    neva_app_runtime::Shell::Shutdown();
  } else {
    for (auto& remote : remotes_)
      remote->OnClose();
  }
}

void PageContentsImpl::OnExit(const std::string& reason) {
  for (auto& remote : remotes_)
    remote->OnExit(reason);
}

void PageContentsImpl::OnFocusChanged(bool is_focused) {
  for (auto& remote : remotes_)
    remote->OnFocusChanged(is_focused);
}

void PageContentsImpl::OnKeyEvent(const std::string& key_name,
                                  int32_t key_code) {
  for (auto& remote : remotes_)
    remote->OnKeyEvent(key_name, key_code);
}

void PageContentsImpl::OnMouseClickEvent(int button_code) {
  for (auto& remote : remotes_)
    remote->OnMouseClickEvent(button_code);
}

void PageContentsImpl::OnNewWindowOpen(
    std::unique_ptr<neva_app_runtime::PageContents> new_contents,
    neva_app_runtime::NewWindowInfo& window_info) {
  auto id = new_contents->GetID();
  neva_app_runtime::ShellEnvironment::GetInstance()->SaveDetachedContents(
      std::move(new_contents));

  for (auto& remote : remotes_) {
    auto new_window_info = browser_shell::mojom::WindowInfo::New();
    new_window_info->target_url = window_info.target_url;
    new_window_info->initial_width = window_info.initial_width;
    new_window_info->initial_height = window_info.initial_height;
    new_window_info->name = window_info.name;
    new_window_info->window_open_disposition =
        window_info.window_open_disposition;
    new_window_info->user_gesture = window_info.user_gesture;
    remote->OnNewWindowOpen(id, std::move(new_window_info));
  }
}

void PageContentsImpl::OnOpenURLFromTab(
    neva_app_runtime::OpenURLInfo& open_info) {
  for (auto& remote : remotes_) {
    auto open_url_info = browser_shell::mojom::OpenURLInfo::New();
    open_url_info->target_url = open_info.target_url;
    open_url_info->window_open_disposition = open_info.window_open_disposition;
    open_url_info->user_gesture = open_info.user_gesture;
    remote->OnOpenURLFromTab(std::move(open_url_info));
  }
}

void PageContentsImpl::OnRendererUnresponsive() {
  for (auto& remote : remotes_)
    remote->OnRendererUnresponsive();
}

void PageContentsImpl::OnRendererResponsive() {
  for (auto& remote : remotes_)
    remote->OnRendererResponsive();
}

void PageContentsImpl::OnPermissionRequest(const std::string& permission,
                                           uint64_t id) {
  for (auto& remote : remotes_)
    remote->OnPermissionRequest(permission, id);
}

void PageContentsImpl::OnTouchStartEvent() {
  for (auto& remote : remotes_)
    remote->OnTouchStartEvent();
}

void PageContentsImpl::OnVisibleRegionCaptured(const std::string& base64_data) {
  if (capture_visible_region_callback_.get() != nullptr) {
    std::move(*capture_visible_region_callback_).Run(base64_data);
    capture_visible_region_callback_.reset();
  }
}

void PageContentsImpl::OnZoomFactorChanged(double zoom_factor) {
  for (auto& remote : remotes_)
    remote->OnZoomFactorChanged(zoom_factor);
}

bool PageContentsImpl::RunJSDialog(const std::string& type,
                                   const std::string& message) {
  auto* parent_page_view = page_contents_->GetParentPageView();
  if (parent_page_view && parent_page_view->GetParentShellWindow()) {
    // That means that the JS Dialog request is done by main application page.
    // alert, confirm and prompt are modal, so the JS in main page is now frozen
    // and we cannot ask it to process any event.
    return false;
  }

  for (auto& remote : remotes_)
    remote->RunJSDialog(type, message);

  return !remotes_.empty();
}

void PageContentsImpl::TitleUpdated(const std::string& title) {
  for (auto& remote : remotes_)
    remote->TitleUpdated(title);
}

void PageContentsImpl::OnDestroying(neva_app_runtime::PageContents* contents) {
  page_contents_ = nullptr;
}

bool PageContentsImpl::GetErrorPageHiding() const {
  return page_contents_->GetErrorPageHiding();
}

}  // namespace browser_shell
