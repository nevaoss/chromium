// Copyright 2016 LG Electronics, Inc.
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

#ifndef NEVA_APP_RUNTIME_WEBVIEW_H_
#define NEVA_APP_RUNTIME_WEBVIEW_H_

#include <optional>
#include <set>

#include "base/memory/memory_pressure_listener.h"
#include "base/memory/weak_ptr.h"
#include "components/performance_manager/public/freezing/freezing.h"
#include "content/public/browser/web_contents_observer.h"
#include "net/cookies/canonical_cookie.h"
#include "neva/app_runtime/browser/app_runtime_web_contents_delegate.h"
#include "neva/app_runtime/public/app_runtime_constants.h"
#include "third_party/blink/public/common/mediastream/media_stream_request.h"
#include "third_party/blink/public/mojom/peerconnection/peer_connection_tracker.mojom-shared.h"
#include "ui/gfx/geometry/size.h"

namespace content {
class WebContents;
}  // namespace content

namespace blink {
namespace web_pref {
struct WebPreferences;
}
}  // namespace blink

namespace neva_app_runtime {

class AppRuntimeEvent;
class AppRuntimeWebViewControllerImpl;
class AppRuntimeWebViewHostImpl;
class WebAppInjectionManager;
class WebViewControllerDelegate;
class WebViewDelegate;
class WebViewProfile;

namespace {
struct WebViewParams {
  std::string app_path;
  std::string trust_level;
};
}  // namespace

class WebView : public AppRuntimeWebContentsDelegate,
                public content::WebContentsObserver {
 public:
  enum Attribute {
    AllowRunningInsecureContent,
    AllowScriptsToCloseWindows,
    AllowUniversalAccessFromFileUrls,
    RequestQuotaEnabled,
    SuppressesIncrementalRendering,
    DisallowScrollbarsInMainFrame,
    DisallowScrollingInMainFrame,
    JavascriptCanOpenWindows,
    SpatialNavigationEnabled,
    SupportsMultipleWindows,
    CSSNavigationEnabled,
    V8DateUseSystemLocaloffset,
    LocalStorageEnabled,
    WebSecurityEnabled,
    XFrameOptionsCrossOriginAllowed,
    KeepAliveWebApp,
    AdditionalFontFamilyEnabled,
    BackHistoryKeyDisabled,
  };

  enum FontFamily {
    StandardFont,
    FixedFont,
    SerifFont,
    SansSerifFont,
    CursiveFont,
    FantasyFont
  };

  static void SetFileAccessBlocked(bool blocked);

  WebView(int width, int height, WebViewProfile* profile = nullptr);
  ~WebView() override;

  void CreateRenderView();
  void SetDelegate(WebViewDelegate* delegate);
  void SetControllerDelegate(WebViewControllerDelegate* delegate);

  void CreateWebContents();
  content::WebContents* GetWebContents();
  void AddUserStyleSheet(const std::string& sheet);
  std::string UserAgent() const;
  void LoadUrl(const GURL& url);
  void StopLoading();
  void LoadExtension(const std::string& name);
  void ClearExtensions();
  void ReplaceBaseURL(const std::string& new_url);
  void SendWebViewInfo(const std::string& app_path,
                       const std::string& trust_level);
  const std::string& GetUrl();

  void SuspendPaintingAndSetVisibilityHidden();
  void ResumePaintingAndSetVisibilityVisible();
  bool SetSkipFrame(bool enable);

  bool IsActiveOnNonBlankPaint() const {
    return active_on_non_blank_paint_;
  }

  void CommitLoadVisually();
  std::string DocumentTitle() const;
  void RunJavaScript(const std::string& js_code);
  void RunJavaScriptInAllFrames(const std::string& js_code);
  void Reload();
  int RenderProcessPid() const;
  bool IsDrmEncrypted(const std::string& url);
  std::string DecryptDrm(const std::string& url);

  int DevToolsPort() const;
  void SetInspectable(bool enable);
  void AddAvailablePluginDir(const std::string& directory);
  void AddCustomPluginDir(const std::string& directory);
  void SetBackgroundColor(int r, int g, int b, int a);
  void SetAllowFakeBoldText(bool allow);
  void SetShouldSuppressDialogs(bool suppress);
  void SetAppId(const std::string& app_id);

  // SetSecurityOrigin is used for changing the security origin for local
  // URLs (kFileScheme). It's needed to set the unique application origin
  // of local storage. This works only for renderer per application model.
  void SetSecurityOrigin(const std::string& identifier);
  void SetAcceptLanguages(const std::string& languages);
  void SetUseLaunchOptimization(bool enabled, int delay_ms);
  void SetUseEnyoOptimization(bool enabled);
  void SetBlockWriteDiskcache(bool blocked);
  void SetTransparentBackground(bool enable);
  void SetBoardType(const std::string& board_type);
  void SetMediaCodecCapability(const std::string& capability);
  void SetMediaPreferences(const std::string& preferences);
  void SetSearchKeywordForCustomPlayer(bool enabled);
  void SetSupportDolbyHDRContents(bool support);
  void SetUseUnlimitedMediaPolicy(bool enabled);
  void UpdatePreferencesAttribute(WebView::Attribute attribute, bool enable);
  void SetNetworkQuietTimeout(double timeout);
  void SetFontFamily(WebView::FontFamily fontFamily, const std::string& font);
  void SetActiveOnNonBlankPaint(bool active);
  void SetViewportSize(int width, int height);
  void NotifyMemoryPressure(base::MemoryPressureLevel level);
  void SetVisible(bool visible);
  void SetDatabaseIdentifier(const std::string& identifier);
  void SetVisibilityState(WebPageVisibilityState visibility_state);
  void DeleteWebStorages(const std::string& identifier);
  void SetFocus(bool focus);
  double GetZoomFactor();
  void SetZoomFactor(double factor);
  void SetDoNotTrack(bool dnt);
  void ForwardAppRuntimeEvent(AppRuntimeEvent* event);
  bool CanGoBack() const;
  void GoBack();
  void SetEnableHtmlSystemKeyboardAttr(bool enable);

  void RequestInjectionLoading(const std::string& injection_name);
  void RequestClearInjections();
  bool IsKeyboardVisible() const;
  void DropAllPeerConnections(
      neva_app_runtime::DropPeerConnectionReason reason);
  void SetV8SnapshotPath(const std::string& v8_snapshot_path);
  void SetV8ExtraFlags(const std::string& v8_extra_flags);
  void ReloadDiscardedWebContents();

  // content::WebContentsDelegate implementation
  content::WebContents* OpenURLFromTab(
      content::WebContents* source,
      const content::OpenURLParams& params,
      base::OnceCallback<void(content::NavigationHandle&)>
          navigation_handle_callback) override;

  void NavigationStateChanged(content::WebContents* source,
                              content::InvalidateTypes changed_flags) override;
  void CloseContents(content::WebContents* source) override;

  bool ShouldSuppressDialogs(content::WebContents* source) override;

  void DidFrameFocused() override;
  bool GetAllowLocalResourceLoad() const override;
  void SetAllowLocalResourceLoad(bool enable);
  void UpdatePreferences();

  void EnterFullscreenModeForTab(
      content::RenderFrameHost* requesting_frame,
      const blink::mojom::FullscreenOptions& options) override;

  void ExitFullscreenModeForTab(content::WebContents* web_contents) override;
  bool IsFullscreenForTabOrPending(
      const content::WebContents* web_contents) override;

  bool CheckMediaAccessPermission(content::RenderFrameHost* render_frame_host,
                                  const url::Origin& security_origin,
                                  blink::mojom::MediaStreamType type) override;

  void RequestMediaAccessPermission(
      content::WebContents* web_contents,
      const content::MediaStreamRequest& request,
      content::MediaResponseCallback callback) override;

  void OverrideWebkitPrefs(blink::web_pref::WebPreferences* prefs) override;

  bool DecidePolicyForResponse(bool is_main_frame,
                               int status_code,
                               const std::string& url,
                               const std::string& status_text) override;

  // content::WebContentsObserver implementation
  void RenderFrameCreated(content::RenderFrameHost* render_frame_host) override;
  void DidStartLoading() override;
  void DidStopLoading() override;
  void DidFinishLoad(content::RenderFrameHost* render_frame_host,
                     const GURL& validated_url) override;
  void DidUpdateFaviconURL(
      content::RenderFrameHost* rfh,
      const std::vector<blink::mojom::FaviconURLPtr>& candidates,
      blink::mojom::FaviconUpdateReason reason) override;
  void DidStartNavigation(content::NavigationHandle* navigation_handle) override;
  void DidFinishNavigation(content::NavigationHandle* navigation_handle) override;
  void DidFailLoad(content::RenderFrameHost* render_frame_host,
                   const GURL& validated_url,
                   int error_code) override;
  void LoadProgressChanged(double progress) override;

  void RenderProcessCreated(base::ProcessHandle handle) override;
  void PrimaryMainFrameRenderProcessGone(
      base::TerminationStatus status) override;
  void DOMContentLoaded(content::RenderFrameHost* frame_host) override;
  void DidDropAllPeerConnections(
      blink::mojom::DropPeerConnectionReason reason) override;
  void AboutToBeDiscarded(content::WebContents* new_contents) override;

  // AppRuntimeWebContentsDelegate implementation
  void SetSSLCertErrorPolicy(SSLCertErrorPolicy policy) override;
  SSLCertErrorPolicy GetSSLCertErrorPolicy() const override;

  // Profile
  WebViewProfile* GetProfile() const;
  void SetProfile(WebViewProfile* profile);

  // For access to WebViewInfo of related webapp
  const WebViewDelegate* GetWebViewDelegate() const { return webview_delegate_; }

  std::unique_ptr<content::WebContents> DiscardWebContents(
      std::unique_ptr<content::WebContents> new_contents);

  PageLifecycleState GetPageLifecycleState();

 private:
  void SendGetCookiesResponse(const net::CookieAccessResultList& cookie_list,
                              const net::CookieAccessResultList& excluded_cookies);

  void UpdatePreferencesAttributeForPrefs(blink::web_pref::WebPreferences* prefs,
                                          WebView::Attribute attribute,
                                          bool enable);

  void NotifyRenderWidgetWasResized();

  void UpdateViewportScaleFactor();
  void GrantLoadLocalResources();
  void FinishLoadCallback(const std::string& url);
  void DispatchNetError(const GURL& url, int error_code);
  void SwitchFullscreenModeForTab(content::WebContents* web_contents,
                                  bool enter_fullscreen);

  raw_ptr<WebViewDelegate> webview_delegate_ = nullptr;
  raw_ptr<WebViewControllerDelegate> webview_controller_delegate_ = nullptr;

  std::unique_ptr<content::WebContents> web_contents_;
  std::unique_ptr<blink::web_pref::WebPreferences> web_preferences_;
  std::unique_ptr<WebAppInjectionManager> injection_manager_;

  bool should_suppress_dialogs_ = false;
  bool active_on_non_blank_paint_ = false;
  bool full_screen_ = false;
  bool enable_skip_frame_ = false;
  bool allow_local_resources_load_ = false;
  bool is_discarded_ = false;
  int width_;
  int height_;
  gfx::Size viewport_size_;
  std::string document_title_;
  std::set<std::string> injected_css_;
  SSLCertErrorPolicy ssl_cert_error_policy_ = SSL_CERT_ERROR_POLICY_DEFAULT;
  raw_ptr<WebViewProfile> profile_ = nullptr;
  std::map<WebView::Attribute, bool> webview_preferences_list_;
  std::optional<performance_manager::freezing::FreezingVote> freezing_vote_;
  WebViewParams webview_params_;

  base::WeakPtrFactory<WebView> weak_factory_{this};
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_WEBVIEW_H_
