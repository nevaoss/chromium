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

#ifndef NEVA_BROWSER_SHELL_SERVICE_BROWSER_SHELL_PAGE_CONTENTS_IMPL_H_
#define NEVA_BROWSER_SHELL_SERVICE_BROWSER_SHELL_PAGE_CONTENTS_IMPL_H_

#include "base/memory/raw_ptr.h"
#include "mojo/public/cpp/bindings/associated_remote.h"
#include "mojo/public/cpp/bindings/pending_associated_remote.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/remote_set.h"
#include "neva/app_runtime/app/app_runtime_page_contents_delegate.h"
#include "neva/browser_shell/service/public/mojom/browser_shell_page_contents.mojom.h"

namespace neva_app_runtime {
class PageContents;
}

namespace browser_shell {

class ShellServiceImpl;

class PageContentsImpl : public mojom::PageContents,
                         public neva_app_runtime::PageContentsDelegate {
 public:
  PageContentsImpl(ShellServiceImpl* shell_service,
                   neva_app_runtime::PageContents* page_contents);
  PageContentsImpl(ShellServiceImpl* service, uint64_t id);
  PageContentsImpl(const PageContentsImpl&) = delete;
  PageContentsImpl& operator=(const PageContentsImpl&) = delete;
  ~PageContentsImpl() override;

  bool GetActiveState();
  uint64_t GetID() const;
  std::string GetUserAgent() const;
  double GetZoomFactor() const;
  neva_app_runtime::PageContents* GetPageContents() const;
  mojo::PendingAssociatedReceiver<mojom::PageContentsClient>
  BindNewEndpointAndPassReceiver();

  // mojom::PageContents
  void RegisterClient(
      mojo::PendingAssociatedRemote<mojom::PageContentsClient> remote) override;

  void Activate() override;
  void AckPermission(bool ack, uint64_t id) override;
  void AckAuthChallenge(const std::string& login,
                        const std::string& passwd,
                        const std::string& url) override;
  void CaptureVisibleRegion(const std::string& format,
                            int32_t quality,
                            int32_t output_width,
                            int32_t output_height,
                            CaptureVisibleRegionCallback callback) override;
  void BindNewPageContentsById(
      mojo::PendingReceiver<mojom::PageContents> receiver,
      uint64_t id,
      BindNewPageContentsByIdCallback callback) override;
  void ClearData(const std::string& clear_options,
                 const std::string& clear_data_type_set) override;
  void Deactivate() override;
  void SyncId(SyncIdCallback callback) override;
  void CloseJSDialog(bool success, const std::string& response) override;
  void ExecuteJavaScriptInAllFrames(const std::string& code) override;
  void ExecuteJavaScriptInMainFrame(const std::string& code) override;
  void ExitFullscreen() override;
  void SyncActiveState(SyncActiveStateCallback callback) override;
  void GoBack() override;
  void GoForward() override;
  void LoadURL(const std::string& url, LoadURLCallback callback) override;
  void Reload() override;
  void ResumeDOM() override;
  void ResumeMedia() override;
  void ScrollByY(int y_shift) override;
  void SetAcceptedLanguages(const std::string& languages) override;
  void SetErrorPageHiding(bool enable) override;
  void SetFocus() override;
  void SetKeyCodesFilter(const std::vector<std::string>& key_codes) override;
  void SetPageBaseBackgroundColor(const std::string& color) override;
  void SetSuppressSSLErrorPolicy(bool suppress_errors) override;
  void SetZoomFactor(double factor) override;
  void Stop() override;
  void SuspendDOM() override;
  void SuspendMedia() override;

  // neva_app_runtime::PageContentsDelegate
  void ActivateContents() override;
  void EnterHtmlFullscreen() override;
  void DidFailLoad(const std::string& url,
                   bool is_main_frame,
                   const std::string& error,
                   int error_code) override;
  void DidFinishLoad(const std::string& url, bool is_main_frame) override;
  void DidFinishNavigation(const std::string& url) override;
  void DidPushHistoryNavigation(const std::string& url) override;
  void DidStartLoading() override;
  void DidStartNavigation(const std::string& url) override;
  void DidStopLoading() override;
  void DidUpdateFaviconUrl(
      const std::vector<neva_app_runtime::FaviconInfo>& info) override;
  void DOMReady() override;
  void LeaveHtmlFullscreen() override;
  void LoadProgressChanged(uint32_t progress) override;
  void NavigationEntryCommitted() override;
  void OnAcceptedLanguagesChanged(const std::string& languages) override;
  void OnAuthChallenge(neva_app_runtime::AuthChallengeInfo& challenge) override;
  void OnCapturingMediaStreamChanged(bool is_capturing_media_stream) override;
  void OnClose() override;
  void OnExit(const std::string& reason) override;
  void OnFocusChanged(bool is_focused) override;
  void OnKeyEvent(const std::string& key_name, int32_t key_code) override;
  void OnMouseClickEvent(int button_code) override;
  void OnNewWindowOpen(
      std::unique_ptr<neva_app_runtime::PageContents> new_contents,
      neva_app_runtime::NewWindowInfo& window_info) override;
  void OnOpenURLFromTab(neva_app_runtime::OpenURLInfo& open_info) override;
  void OnRendererUnresponsive() override;
  void OnRendererResponsive() override;
  void OnPermissionRequest(const std::string& permission, uint64_t id) override;
  void OnTouchStartEvent() override;
  void OnVisibleRegionCaptured(const std::string& base64_data) override;
  void OnZoomFactorChanged(double zoom_factor) override;
  bool RunJSDialog(const std::string& type,
                   const std::string& message) override;
  void TitleUpdated(const std::string& title) override;
  void OnDestroying(neva_app_runtime::PageContents* contents) override;

  bool GetErrorPageHiding() const;

 protected:
  const raw_ptr<ShellServiceImpl> shell_service_;

 private:
  raw_ptr<neva_app_runtime::PageContents> page_contents_;
  mojo::AssociatedRemoteSet<mojom::PageContentsClient> remotes_;
  std::unique_ptr<CaptureVisibleRegionCallback>
      capture_visible_region_callback_;
};

}  // namespace browser_shell

#endif  // NEVA_BROWSER_SHELL_SERVICE_BROWSER_SHELL_PAGE_CONTENTS_IMPL_H_
