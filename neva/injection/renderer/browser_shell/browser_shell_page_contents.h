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
#ifndef NEVA_INJECTION_RENDERER_BROWSER_SHELL_BROWSER_SHELL_PAGE_CONTENTS_H_
#define NEVA_INJECTION_RENDERER_BROWSER_SHELL_BROWSER_SHELL_PAGE_CONTENTS_H_

#include <string>

#include "base/memory/weak_ptr.h"
#include "gin/object_template_builder.h"
#include "gin/wrappable.h"
#include "mojo/public/cpp/bindings/associated_receiver.h"
#include "mojo/public/cpp/bindings/pending_associated_receiver.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "neva/browser_shell/service/public/mojom/browser_shell_page_contents.mojom.h"
#include "neva/browser_shell/service/public/mojom/browser_shell_service.mojom.h"
#include "neva/injection/renderer/browser_shell/browser_shell_dialog_controller.h"
#include "neva/injection/renderer/browser_shell/browser_shell_login.h"
#include "neva/injection/renderer/browser_shell/browser_shell_permission_request.h"
#include "neva/injection/renderer/injection_events_emitter.h"
#include "v8/include/v8.h"

namespace blink {
class WebLocalFrame;
}

namespace gin {
class Arguments;
}

namespace injections {

class BrowserShellPageContents
    : public gin::Wrappable<BrowserShellPageContents>,
      public InjectionEventsEmitter<BrowserShellPageContents>,
      public browser_shell::mojom::PageContentsClient,
      public DialogController::Delegate,
      public BrowserShellLogin::Delegate,
      public PermissionRequest::Delegate {
 public:
  static gin::WrapperInfo kWrapperInfo;

  static char kIdPropertyName[];
  static char kCanGoBackPropertyName[];
  static char kCanGoForwardPropertyName[];
  static char kErrorPageHidingPropertyName[];
  static char kIsActivePropertyName[];
  static char kIsAlivePropertyName[];
  static char kUrlPropertyName[];
  static char kUserAgentPropertyName[];
  static char kZoomFactorProperyName[];
  static char kActivateMethodName[];
  static char kCaptureVisibleRegionMethodName[];
  static char kCloseNowMethodName[];
  static char kClearDataMethodName[];
  static char kDeactivateMethodName[];
  static char kEnableHandShapedCursorForLinksMethodName[];
  static char kEnableKeyCodeEventMethodName[];
  static char kExecuteJavaScriptInAllFramesMethodName[];
  static char kExecuteJavaScriptInMainFrameMethodName[];
  static char kExitFullscreenMethodName[];
  static char kGoBackMethodName[];
  static char kGoForwardMethodName[];
  static char kLoadFileMethodName[];
  static char kLoadURLMethodName[];
  static char kReloadMethodName[];
  static char kResumeDOMMethodName[];
  static char kResumeMediaMethodName[];
  static char kScrollByYMethodName[];
  static char kSetAcceptedLanguagesMethodName[];
  static char kSetFocusMethodName[];
  static char kSetPageBaseBackgroundColorMethodName[];
  static char kSetSuppressSSLErrorPolicyMethodName[];
  static char kStopMethodName[];
  static char kSuspendDOMMethodName[];
  static char kSuspendMediaMethodName[];

  static void ConstructorCallback(
      mojo::Remote<browser_shell::mojom::ShellService>* shell_service,
      gin::Arguments* args);

  static v8::Local<v8::Object> BuildOptions(v8::Isolate* isolate,
                                            v8::Local<v8::Value> json_value);
  struct CreateParams {
    bool error_page_hiding = false;
    bool is_main_contents = false;
  };

  BrowserShellPageContents(
      v8::Isolate* isolate,
      mojo::Remote<browser_shell::mojom::PageContents> remote,
      const CreateParams& params,
      uint64_t id = 0);
  BrowserShellPageContents(const BrowserShellPageContents&) = delete;
  BrowserShellPageContents& operator=(const BrowserShellPageContents&) = delete;
  ~BrowserShellPageContents() override;

  base::WeakPtr<BrowserShellPageContents> GetAsWeakPtr() {
    return weak_ptr_factory_.GetWeakPtr();
  }

  uint64_t GetID();
  void Setup(uint64_t id,
             browser_shell::mojom::PageContentsCreationInfoPtr info);
  void OnLoadURL(const std::string& url);

  // ObjectTemplateBuilder::SetMethod does not support exposing inherited
  // methods. Such proxy-calling inherited methods is ugly but the easiest
  // workaround.
  void RunGetEventNames(gin::Arguments* args) const {
    InjectionEventsEmitter::GetEventNames(args);
  }
  void RunEmit(gin::Arguments* args) { InjectionEventsEmitter::Emit(args); }
  void RunAddEventListener(gin::Arguments* args) {
    InjectionEventsEmitter::AddEventListener(args);
  }
  void RunAddOnceEventListener(gin::Arguments* args) {
    InjectionEventsEmitter::AddOnceEventListener(args);
  }
  int RunGetListenerCount(const std::string& name) const {
    return InjectionEventsEmitter::GetListenerCount(name);
  }
  void RunRemoveEventListener(gin::Arguments* args) {
    InjectionEventsEmitter::RemoveEventListener(args);
  }
  void RunRemoveAllEventListeners(gin::Arguments* args) {
    InjectionEventsEmitter::RemoveAllEventListeners(args);
  }

  // browser_shell::mojom::PageContentsClient
  void ActivateContents() override;
  void EnterHtmlFullscreen() override;
  void DidFailLoad(const std::string& url,
                   bool is_main_frame,
                   const std::string& error,
                   int32_t error_code) override;
  void DidFinishLoad(const std::string& url, bool is_main_frame) override;
  void DidFinishNavigation(const std::string& url) override;
  void DidPushHistoryNavigation(const std::string& url) override;
  void DidStartLoading() override;
  void DidStartNavigation(const std::string& url) override;
  void DidStopLoading() override;
  void DidUpdateFaviconUrl(
      std::vector<browser_shell::mojom::FaviconInfoPtr> favicons) override;
  void DOMReady() override;
  void LeaveHtmlFullscreen() override;
  void LoadProgressChanged(uint32_t progress) override;
  void OnAcceptedLanguagesChanged(const std::string& languages) override;
  void OnAuthChallenge(
      browser_shell::mojom::AuthChallengePtr challenge) override;
  void OnCapturingMediaStreamChanged(bool is_capturing_media_stream) override;
  void OnClose() override;
  void OnExit(const std::string& reason) override;
  void OnFocusChanged(bool is_focused) override;
  void OnKeyEvent(const std::string& key_name, int32_t key_code) override;
  void OnMouseClickEvent(int button_code) override;
  void OnNavigationEntryCommitted(
      browser_shell::mojom::PageContentsStatePtr state) override;
  void OnNewWindowOpen(
      uint64_t id,
      browser_shell::mojom::WindowInfoPtr window_info) override;
  void OnOpenURLFromTab(
      browser_shell::mojom::OpenURLInfoPtr open_url_info) override;
  void OnRendererUnresponsive() override;
  void OnRendererResponsive() override;
  void OnPermissionRequest(const std::string& permission, uint64_t id) override;
  void OnTouchStartEvent() override;
  void OnZoomFactorChanged(double zoom_factor) override;
  void RunJSDialog(const std::string& type,
                   const std::string& message) override;
  void TitleUpdated(const std::string& title) override;

  // Override BrowserShellLogin::Delegate
  void AckAuthChallenge(const std::string& login,
                        const std::string& passwd,
                        const std::string& url) override;

  // Override PermissionRequest::Delegate
  void AckPermission(PermissionRequest::Decision result,
                     uint64_t request_id,
                     uint64_t page_id) override;

  // DialogController::Delegate
  void CloseJSDialog(bool success, const std::string& response) override;

 private:
  struct State {
    bool can_go_back = false;
    bool can_go_forward = false;
  };

  void Activate();
  void CaptureVisibleRegion(gin::Arguments* args);
  void ClearData(gin::Arguments* args);
  void Deactivate();
  void ExitFullscreen();
  bool GetActiveState();
  bool GetErrorPageHiding() const;
  void EnableHandShapedCursorForLinks();
  void ExecuteJavaScriptInAllFrames(const std::string& code);
  void ExecuteJavaScriptInMainFrame(const std::string& code);
  bool CanGoBack() const;
  bool CanGoForward() const;
  void CloseNow();
  std::string GetUrl() const;
  std::string GetUserAgent() const;
  double GetZoomFactor() const;
  void GoBack();
  void GoForward();
  bool IsAlive() const;
  void LoadFile(gin::Arguments* args);
  void LoadURL(const std::string& url);
  void Reload();
  void ResumeDOM();
  void ResumeMedia();
  void ScrollByY(int y_shift);
  void SetAcceptedLanguages(const std::string& languages);
  void SetErrorPageHiding(bool enable);
  void SetFocus();
  void SetKeyCodesFilter(gin::Arguments* args);
  // color is a RGBA or RGB string like #FFFFFFFF, #FFFFFF, #FFFF, #FFF
  void SetPageBaseBackgroundColor(const std::string& color);
  void SetSuppressSSLErrorPolicy(bool suppress_errors);
  void SetZoomFactor(double factor);
  void Stop();
  void SuspendDOM();
  void SuspendMedia();

  void OnCaptureVisibleRegionReply(
      std::unique_ptr<v8::Persistent<v8::Function>> callback,
      const std::string& base64_data);

  gin::ObjectTemplateBuilder GetObjectTemplateBuilder(
      v8::Isolate* isolate) override;

  uint64_t id_;
  bool error_page_hiding_;
  std::string user_agent_;
  bool is_main_contents_;
  bool is_active_;
  State state_;
  std::string url_string_;
  double zoom_factor_;

  mojo::Remote<browser_shell::mojom::PageContents> remote_;
  mojo::AssociatedReceiver<browser_shell::mojom::PageContentsClient> receiver_;
  base::WeakPtrFactory<BrowserShellPageContents> weak_ptr_factory_{this};
};

}  // namespace injections

#endif  // NEVA_INJECTION_RENDERER_BROWSER_SHELL_BROWSER_SHELL_PAGE_CONTENTS_H_
