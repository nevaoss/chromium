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

#ifndef NEVA_APP_RUNTIME_PUBLIC_WEBVIEW_BASE_H_
#define NEVA_APP_RUNTIME_PUBLIC_WEBVIEW_BASE_H_

#include <memory>
#include <string>

#include "neva/app_runtime/public/app_runtime_constants.h"
#include "neva/app_runtime/public/app_runtime_export.h"
#include "neva/app_runtime/public/webview_base_internals.h"
#include "neva/app_runtime/public/webview_controller_delegate.h"
#include "neva/app_runtime/public/webview_delegate.h"
#include "neva/app_runtime/public/webview_info.h"

namespace content {
class ScopedAccessibilityMode;
}

namespace neva_app_runtime {

class AppRuntimeEvent;
class WebView;
class WebViewBaseObserver;
class WebViewProfile;

/**
 * Historically, WebView is a GUI widget that represents a browser without
 * the address line and the navigation pane rendering a web page addressed by
 * an URL pointed by the application developer.
 *
 * Current implementation of this entity is an abstract C++ class. It means
 * that the class contains methods that are defined and pure virtual methods
 * that are to be defined. In order to instantiate an object of the class
 * the developer has to implement a derived class, defining all the pure
 * virtual methods. Hence, the word 'Base' in the class name.
 *
 * Calls of defined methods are intended to be used to control the WebView
 * entity. The pure virtual methods that are to be defined in the derived
 * class will be called by the web engine in order communicate events to the
 * application. Thus, an object-oriented approach similar to C-style
 * callbacks shall be used.
 *
 * The resulting derived class will be represent the WebView entity that can
 * be used for application development on the target platform.
 */
class APP_RUNTIME_EXPORT WebViewBase : public WebViewDelegate,
                                       public WebViewControllerDelegate,
                                       public internals::WebViewBaseInternals {
 public:
  enum MemoryPressureLevel {
    MEMORY_PRESSURE_NONE = 0,
    MEMORY_PRESSURE_LOW = 1,
    MEMORY_PRESSURE_CRITICAL = 2
  };

  static void SetFileAccessBlocked(bool blocked);

  WebViewBase(int width = 1920,
              int height = 1080,
              WebViewProfile* profile = nullptr);
  ~WebViewBase() override;

  void CreateRenderView();

  /**
   * @name internals::WebViewBaseInternals-inherited method
   * @{
   */

  /**
   * @return The pointer to the content::WebContents object. WebContents is a
   * Chromium entity that renders web content (usually HTML) in a rectangular
   * area.
   */
  content::WebContents* GetWebContents() override;

  /**
   * @}
   */

  /**
   * @name WebViewControllerDelegate-inherited methods
   * @{
   */

  /**
   * A dummy implementation. Does nothing. Absolutely nothing.
   */
  void RunCommand(const std::string& name,
                  const std::vector<std::string>& arguments) override;

  /**
   * A dummy implementation. Returns an empty string. Absolutely empty string.
   */
  std::string RunFunction(const std::string& name,
                          const std::vector<std::string>& arguments) override;

  /**
   * @}
   */

  /**
   * Apply the style sheet to the web page rendered. According to the
   * cascading scheme of style sheets application the style sheet settings
   * will be applied with least priority.
   *
   * @param sheet The style sheet to be applied.
   */
  void AddUserStyleSheet(const std::string& sheet);

  /**
   * The default value of the user agent string. The default value
   * may be replaced by the overriden value.
   * @see SetUserAgent()
   *
   * @return The user agent string may be passed to the webapp runtime
   * by means of the 'user-agent' command line switch. Otherwise the Google
   * Chrome engine user agent string will be returned.
   */
  std::string DefaultUserAgent() const;

  /**
   * @return The overriden user agent string. In case the user agent string
   * hasn't been overriden, an empty string will be returned.
   *
   * @see SetUserAgent()
   */
  std::string UserAgent() const;

  /**
   * Request to render a web page identified by a URL.
   *
   * @param url The URL of the web page to be rendered.
   */
  void LoadUrl(const std::string& url);

  /**
   * Stops the web page loading.
   */
  void StopLoading();
  void LoadExtension(const std::string& name);

  /**
   * @deprecated Used in some LG Electronics products.
   */
  void ReplaceBaseURL(const std::string& new_url, const std::string& old_url);

  /**
   * Enable DOM inspection for the current WebView. The inspection could
   * be conducted in case it is enabled by means of SetInspectable().
   *
   * @see SetInspectable()
   */
  void EnableInspectablePage();

  /**
   * Disable DOM inspection for the current WebView. In order to disable
   * DOM inspection for all the WebViews hosted on the web engine that
   * hosts current WebView, please call the SetInspectable(false) method.
   * In this case the HTTP server that hosts the DOM inspection data will
   * be shut down.
   *
   * @see SetInspectable()
   */
  void DisableInspectablePage();

  /**
   * In case the DOM inspection is enabled, the method returns the TCP
   * port number of the HTTP server that provides the inspection data.
   *
   * @see SetInspectable()
   *
   * @return The TCP port number of the HTTP server that provides the
   * inspection data.
   */
  int DevToolsPort() const;

  /**
   * Enables/disables DOM inspection for all the WebViews hosted on the web
   * engine instance that hosts the current WebView. When the inspection is
   * enabled the web engine opens an HTTP server on the host machine. The TCP
   * port number for the HTTP server could be then learned by means of the
   * DevToolsPort() method.
   *
   * @see DevToolsPort()
   *
   * @param enable The flag that tells the web engine whether the inspection
   * should be enabled or disabled.
   */
  void SetInspectable(bool enable);
  void AddAvailablePluginDir(const std::string& directory);
  void AddCustomPluginDir(const std::string& directory);

  /**
   * The user agent string may be overriden. This function sets this
   * overriden value of the user agent string.
   *
   * @param useragent The user agent string overriding the default value.
   */
  void SetUserAgent(const std::string& useragent);
  void SetBackgroundColor(int r, int g, int b, int alpha);
  void SetShouldSuppressDialogs(bool suppress);
  void SetUseAccessibility(bool enabled);
  void SetActiveOnNonBlankPaint(bool active);
  void SetViewportSize(int width, int height);
  void NotifyMemoryPressure(MemoryPressureLevel level);
  void SetVisible(bool visible);
  void SetVisibilityState(WebPageVisibilityState visibility_state);
  void DeleteWebStorages(const std::string& identifier);

  /**
   * Get the document title of the current page rendered.
   *
   * @return The document title of the current page rendered.
   */
  std::string DocumentTitle() const;
  void SuspendWebPageDOM();
  void ResumeWebPageDOM();
  void SuspendWebPageMedia();
  void ResumeWebPageMedia();
  void SuspendPaintingAndSetVisibilityHidden();
  void ResumePaintingAndSetVisibilityVisible();
  void CommitLoadVisually();
  void RunJavaScript(const std::string& js_code);
  void RunJavaScriptInAllFrames(const std::string& js_code);

  /**
   * Reloads the current rendered web page.
   */
  void Reload();

  /**
   * Each WebView has a corresponding rendering process in the host operating
   * system that renders the web page presented by it. The method returns the
   * rendering process ID.
   *
   * @return The renderer process ID in the host operating system
   */
  int RenderProcessPid() const;
  bool IsDrmEncrypted(const std::string& url);
  std::string DecryptDrm(const std::string& url);
  void SetFocus(bool focus);
  double GetZoomFactor();
  void SetZoomFactor(double factor);
  void SetDoNotTrack(bool dnt);
  void ForwardAppRuntimeEvent(AppRuntimeEvent* event);

  /**
   * Tells whether there is a previous page in the browsing history.
   *
   * @see GoBack()
   * @return true in case there is a previous page in the browsing history
   *         false in case current page is the first in the browsing history
   */
  bool CanGoBack() const;

  /**
   * Loads the previous page in the browsing history in case one is available.
   * @see CanGoBack()
   *
   */
  void GoBack();
  void SetEnableHtmlSystemKeyboardAttr(bool enabled);
  void RequestInjectionLoading(const std::string& injection_name);
  void RequestClearInjections();
  void DropAllPeerConnections(DropPeerConnectionReason reason);

  /**
   * Get the URL of the current page rendered.
   *
   * @return The URL of the current page rendered.
   */
  const std::string& GetUrl();

  /**
   * Get the Page Lifecycle state of the current page.
   *
   * @return The Page Lifecycle state of the current page.
   */
  PageLifecycleState GetPageLifecycleState();

  // RenderViewHost
  void SetUseLaunchOptimization(bool enabled, int delay_ms);
  void SetUseEnyoOptimization(bool enabled);
  void SetBlockWriteDiskcache(bool blocked);
  void SetTransparentBackground(bool enabled);

  // RenderPreference
  void SetAllowFakeBoldText(bool allow);
  void SetAppId(const std::string& appId);
  void SetSecurityOrigin(const std::string& identifier);
  void SetAcceptLanguages(const std::string& lauguages);
  void SetBoardType(const std::string& board_type);
  void SetMediaCodecCapability(const std::string& capability);
  void SetMediaPreferences(const std::string& preferences);
  void SetSearchKeywordForCustomPlayer(bool enabled);
  void SetSupportDolbyHDRContents(bool support);
  void SetUseUnlimitedMediaPolicy(bool enabled);

  // WebPreferences
  void SetAllowRunningInsecureContent(bool enable);
  void SetAllowScriptsToCloseWindows(bool enable);
  void SetAllowUniversalAccessFromFileUrls(bool enable);
  void SetRequestQuotaEnabled(bool enable);
  void SetSuppressesIncrementalRendering(bool enable);
  void SetDisallowScrollbarsInMainFrame(bool enable);
  void SetDisallowScrollingInMainFrame(bool enable);
  void SetJavascriptCanOpenWindows(bool enable);
  void SetSpatialNavigationEnabled(bool enable);
  void SetSupportsMultipleWindows(bool enable);
  void SetCSSNavigationEnabled(bool enable);
  void SetV8DateUseSystemLocaloffset(bool use);
  void SetAllowLocalResourceLoad(bool enable);
  void SetLocalStorageEnabled(bool enable);
  void SetDatabaseIdentifier(const std::string& identifier);
  void SetBackHistoryKeyDisabled(bool disabled);
  void SetWebSecurityEnabled(bool enable);
  void SetXFrameOptionsCrossOriginAllowed(bool enable);
  void SetKeepAliveWebApp(bool enable);
  void SetAdditionalFontFamilyEnabled(bool enable);
  void SetNetworkQuietTimeout(double timeout);

  // FontFamily
  void SetStandardFontFamily(const std::string& font);
  void SetFixedFontFamily(const std::string& font);
  void SetSerifFontFamily(const std::string& font);
  void SetSansSerifFontFamily(const std::string& font);
  void SetCursiveFontFamily(const std::string& font);
  void SetFantasyFontFamily(const std::string& font);
  void LoadAdditionalFont(const std::string& url, const std::string& font);

  void SetSSLCertErrorPolicy(SSLCertErrorPolicy policy);
  SSLCertErrorPolicy GetSSLCertErrorPolicy();

  // Profile
  WebViewProfile* GetProfile() const;
  void SetProfile(WebViewProfile* profile);

  // WebViewDelegate
  const WebViewInfo& GetWebViewInfo() const override;

 private:
  WebView* webview_;
  WebViewInfo webview_info_;
  std::unique_ptr<content::ScopedAccessibilityMode> scoped_accessibility_mode_;
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_PUBLIC_WEBVIEW_BASE_H_
