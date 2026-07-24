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

#ifndef NEVA_APP_RUNTIME_APP_APP_RUNTIME_PAGE_CONTENTS_H_
#define NEVA_APP_RUNTIME_APP_APP_RUNTIME_PAGE_CONTENTS_H_

#include <map>
#include <memory>
#include <optional>
#include <string>

#include "base/callback_list.h"
#include "base/memory/weak_ptr.h"
#include "components/performance_manager/public/freezing/freezing.h"
#include "components/sessions/core/serialized_navigation_entry.h"
#include "content/public/browser/host_zoom_map.h"
#include "content/public/browser/web_contents_observer.h"
#include "net/base/auth.h"
#include "net/base/network_anonymization_key.h"
#include "neva/app_runtime/app/app_runtime_js_dialog_manager_delegate.h"
#include "neva/app_runtime/app/app_runtime_page_contents_delegate.h"
#include "neva/app_runtime/browser/app_runtime_web_contents_delegate.h"
#include "neva/app_runtime/public/app_runtime_constants.h"
#include "third_party/blink/public/mojom/mediastream/media_stream.mojom.h"
#include "third_party/blink/public/mojom/window_features/window_features.mojom.h"

namespace content {
class WebContents;
struct MediaStreamRequest;
}  // namespace content

namespace net {
class AuthChallengeInfo;
class NetworkAnonymizationKey;
}  // namespace net

namespace neva_app_runtime {

class JSDialogManager;
class PageView;
class VisibleRegionCapture;
class WebAppInjectionManager;

class PageContents : public AppRuntimeWebContentsDelegate,
                     public content::WebContentsObserver,
                     public JSDialogManagerDelegate {
 public:
  enum class Type {
    kTab = 0,
    kExtensionPopup,
    kExtensionPopupWindow,
    kExtensionNormal,
    kUI,
    kMain,
    kLast = kMain,
    kUnknown,
  };

  static char kTypeTab[];
  static char kTypeExtensionPopup[];
  static char kTypeExtensionPopupWindow[];
  static char kTypeExtensionNormal[];
  static char kTypeUI[];
  static char kTypeMain[];
  static char kTypeUnknown[];

  static bool IsContentType(Type type);
  static bool IsUIType(Type type);
  static bool IsServiceType(Type type);

  static std::string ConvertTypeToString(Type type);
  static Type ConvertStringToType(const std::string& type_str);

  struct CreateParams {
    CreateParams();
    CreateParams(const CreateParams&);
    CreateParams& operator=(const CreateParams&);
    ~CreateParams();

    int width = 0;
    int height = 0;
    raw_ptr<PageContentsDelegate> delegate = nullptr;
    std::map<std::string, std::string> injections;
    bool inspectable = false;
    bool allow_file_access_from_file_urls = false;
    bool allow_universal_access_from_file_urls = false;
    std::string app_id;
    std::string media_codec_capability;
    std::string storage_partition_name;
    bool storage_partition_off_the_record = false;
    std::string user_agent;
    bool active = false;
    bool error_page_hiding = false;
    std::optional<bool> default_access_to_media = std::nullopt;
    std::optional<double> zoom_factor = std::nullopt;
    Type type = Type::kTab;
    std::string accepted_languages;
    std::string v8_snapshot_path;
    std::optional<uint64_t> site_page_contents_id;
    // To allow automatic fullscreen on web contents
    // (e.g. `Element.requestFullscreen()`) value must be false.
    bool require_transient_activation_for_html_fullscreen = true;
  };

  static PageContents* From(content::WebContents* web_contents);
  static PageContents* From(uint64_t id);

  explicit PageContents(const CreateParams& params);
  PageContents(const PageContents&) = delete;
  PageContents& operator=(const PageContents&) = delete;
  ~PageContents() override;

  void SetDelegate(PageContentsDelegate* delegate);
  PageContentsDelegate* GetDelegate() const;

  uint64_t GetID() const;
  Type GetType() const;

  bool IsMain() const { return GetType() == Type::kMain; }

  bool IsTab() const { return GetType() == Type::kTab; }

  bool IsTransientActivationForHtmlFullscreenRequired() const;
  content::WebContents* GetWebContents() const;

  void SetV8SnapshotPath(const std::string& v8_snapshot_path) const;

  void Activate();
  void CaptureVisibleRegion(const std::string& format,
                            int quality = 90,
                            int output_width = 0,
                            int output_height = 0);
  void AckPermission(bool ack, uint64_t id);
  void AckAuthChallenge(const std::string& login,
                        const std::string& passwd,
                        const std::string& url);
  void ClearData(const std::string& clear_options,
                 const std::string& clear_data_type_set);
  void Deactivate(bool by_page_discarder = false);
  void ExecuteJavaScriptInAllFrames(const std::string& code_string);
  void ExecuteJavaScriptInMainFrame(const std::string& code_string);
  void ExitFullscreen();
  bool CanGoBack() const;
  bool CanGoForward() const;
  void CloseJSDialog(bool success, const std::string& response);

  std::string GetAcceptedLanguages() const;
  bool GetErrorPageHiding() const;
  std::string GetUserAgent() const;
  double GetZoomFactor() const;
  void GoBack();
  void GoForward();
  bool IsActive();
  bool LoadURL(std::string url_string);
  bool Reload();
  void ReloadNoWarranty();
  void ResumeDOM();
  void ResumeMedia();
  void ScrollByY(int y_shift);
  void SetAcceptedLanguages(std::string languages);
  void SetErrorPageHiding(bool enable);
  void SetFocus();
  void SetKeyCodesFilter(const std::vector<std::string>& key_codes);
  // color is a RGBA or RGB string like #FFFFFFFF, #FFFFFF, #FFFF or #FFF
  void SetPageBaseBackgroundColor(std::string color);
  void SetZoomFactor(double factor);
  void Stop();
  void SetUserAgentOverride(const std::string& user_agent);
  void SuspendDOM();
  void SuspendMedia();
  void UpdatePreferredLanguage(std::string language);

  PageView* GetParentPageView() const;

  // WebContentsObserver
  void DidFailLoad(content::RenderFrameHost* render_frame_host,
                   const GURL& validated_url,
                   int error_code) override;
  void DidFinishLoad(content::RenderFrameHost* render_frame_host,
                     const GURL& validated_url) override;
  void DidFinishNavigation(
      content::NavigationHandle* navigation_handle) override;
  void DidStartLoading() override;
  void DidStartNavigation(
      content::NavigationHandle* navigation_handle) override;
  void DidStopLoading() override;
  void DidUpdateFaviconURL(
      content::RenderFrameHost* render_frame_host,
      const std::vector<blink::mojom::FaviconURLPtr>& candidates,
      blink::mojom::FaviconUpdateReason reason) override;
  void DidGetUserInteraction(const blink::WebInputEvent& event) override;
  void DOMContentLoaded(content::RenderFrameHost* render_frame_host) override;
  void LoadProgressChanged(double progress) override;
  void NavigationEntryCommitted(
      const content::LoadCommittedDetails& load_details) override;
  void OnFrameIsCapturingMediaStreamChanged(
      content::RenderFrameHost* rfh,
      bool is_capturing_media_stream) override;
  void OnRendererUnresponsive(
      content::RenderProcessHost* render_process_host) override;
  void OnRendererResponsive(
      content::RenderProcessHost* render_process_host) override;
  void OnWebContentsFocused(
      content::RenderWidgetHost* render_widget_host) override;
  void OnWebContentsLostFocus(
      content::RenderWidgetHost* render_widget_host) override;
  void PrimaryMainFrameRenderProcessGone(
      base::TerminationStatus status) override;
  void RenderFrameCreated(content::RenderFrameHost* render_frame_host) override;
  void RenderFrameHostChanged(content::RenderFrameHost* old_host,
                              content::RenderFrameHost* new_host) override;
  void RequestPointerLock(content::WebContents* web_contents,
                          bool user_gesture,
                          bool last_unlocked_by_target) override;

  // AppRuntimeWebContentsDelegate
  void SetSSLCertErrorPolicy(SSLCertErrorPolicy policy) override;
  SSLCertErrorPolicy GetSSLCertErrorPolicy() const override;

  // WebContentsDelegate
  content::WebContents* OpenURLFromTab(
      content::WebContents* source,
      const content::OpenURLParams& params,
      base::OnceCallback<void(content::NavigationHandle&)>
          navigation_handle_callback) override;
  content::JavaScriptDialogManager* GetJavaScriptDialogManager(
      content::WebContents* source) override;
  content::WebContents* AddNewContents(
      content::WebContents* source,
      std::unique_ptr<content::WebContents> new_contents,
      const GURL& target_url,
      WindowOpenDisposition disposition,
      const blink::mojom::WindowFeatures& window_features,
      bool user_gesture,
      bool* was_blocked) override;
  void ActivateContents(content::WebContents* contents) override;
  void CloseContents(content::WebContents* source) override;
  void EnterFullscreenModeForTab(
      content::RenderFrameHost* requesting_frame,
      const blink::mojom::FullscreenOptions& options) override;
  void FullscreenStateChangedForTab(
      content::RenderFrameHost* requesting_frame,
      const blink::mojom::FullscreenOptions& options) override;
  void ExitFullscreenModeForTab(content::WebContents*) override;
  bool IsFullscreenForTabOrPending(
      const content::WebContents* web_contents) override;
  void NavigationStateChanged(content::WebContents* source,
                              content::InvalidateTypes changed_flags) override;
  void RequestMediaAccessPermission(
      content::WebContents* web_contents,
      const content::MediaStreamRequest& request,
      content::MediaResponseCallback callback) override;
  bool CheckMediaAccessPermission(content::RenderFrameHost* render_frame_host,
                                  const url::Origin& security_origin,
                                  blink::mojom::MediaStreamType type) override;
  void OverrideWebkitPrefs(blink::web_pref::WebPreferences* prefs) override;

  // JSDialogManagerDelegate
  bool RunJSDialog(const std::string& type,
                   const std::string& message) override;
 private:
  friend PageView;
  static std::unique_ptr<content::WebContents> CreateWebContents(
      const PageContents::CreateParams& params);
  static std::unique_ptr<content::WebContents> ReCreateWebContents(
      content::BrowserContext* browser_context,
      const content::SessionStorageNamespaceMap& session_storage_namespace,
      scoped_refptr<content::SiteInstance> site_instance);

  PageContents(std::unique_ptr<content::WebContents> new_contents,
               const CreateParams& params);

  void Initialize();
  void SetParentPageView(PageView* page_view);
  void ResetRendererPreferences();
  void RequestAllInjectionsLoading();
  void OnCaptureVisibleRegion(std::string base64);
  void OnZoomLevelChanged(const content::HostZoomMap::ZoomLevelChange& change);

  struct MediaAccessPermissionInfo {
    MediaAccessPermissionInfo();
    MediaAccessPermissionInfo(
        const blink::mojom::StreamDevicesSet& stream_devices_set,
        content::MediaResponseCallback callback);
    MediaAccessPermissionInfo(MediaAccessPermissionInfo&&);
    ~MediaAccessPermissionInfo();

    static uint64_t id;
    blink::mojom::StreamDevicesSetPtr stream_devices_set_;
    content::MediaResponseCallback callback;
  };
  std::map<uint64_t, MediaAccessPermissionInfo> media_access_requests_;

  CreateParams create_params_;
  const uint64_t id_ = 0;
  raw_ptr<PageView> parent_page_view_ = nullptr;
  raw_ptr<PageContentsDelegate> delegate_ = nullptr;
  PageContentsDelegate stub_delegate_;
  bool page_requested_fullscreen_ = false;
  std::unique_ptr<content::WebContents> web_contents_;
  std::optional<bool> default_access_to_media_ = std::nullopt;
  std::unique_ptr<JSDialogManager> js_dialog_manager_;
  std::list<std::string> injections_;
  std::unique_ptr<WebAppInjectionManager> injection_manager_;
  std::string user_agent_;
  SSLCertErrorPolicy ssl_cert_error_policy_ = SSL_CERT_ERROR_POLICY_DEFAULT;
  std::set<uint32_t> key_codes_filter_;
  std::optional<performance_manager::freezing::FreezingVote> freezing_vote_;

  // zoom changing subscription
  raw_ptr<content::HostZoomMap> host_zoom_map_ = nullptr;
  base::CallbackListSubscription zoom_changed_subscription_;
  double zoom_factor_;

  // data for backup session
  content::SessionStorageNamespaceMap session_storage_namespace_map_;
  std::vector<sessions::SerializedNavigationEntry> navigations_;
  int current_navigation_entry_index_;
  raw_ptr<content::BrowserContext> last_browser_context_ = nullptr;
  scoped_refptr<content::SiteInstance> site_instance_;

  bool error_page_hiding_;
  std::unique_ptr<VisibleRegionCapture> visible_region_capture_;

  // login request data
  std::optional<net::AuthChallengeInfo> auth_challenge_;
  net::NetworkAnonymizationKey network_anonymization_key_;
  base::WeakPtrFactory<PageContents> weak_ptr_factory_{this};
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_APP_APP_RUNTIME_PAGE_CONTENTS_H_
