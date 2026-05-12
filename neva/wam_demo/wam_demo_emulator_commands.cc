// Copyright 2020 LG Electronics, Inc.
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

#include "neva/wam_demo/wam_demo_emulator_commands.h"

namespace wam_demo {
namespace command {

const char kAddUserStyleSheet[] = "addUserStyleSheet";
const char kAllowLocalResourceLoad[] = "allowLocalResourceLoad";
const char kAllowUniversalAccessFromFileUrls[] = "allowUniversalAccessFromFileUrls";
const char kAllowXFrameSameOrigin[] = "allowXFrameSameOrigin";
const char kCanGoBack[] = "canGoBack";
const char kChangeUrl[] = "changeURL";
const char kClearBrowsingData[] = "clearBrowsingData";
const char kClearInjections[] = "clearInjections";
const char kClearMediaCapturePermission[] = "clearMediaCapturePermission";
const char kDecidePolicyForResponse[] = "decidePolicyForResponse";
const char kDeleteWebStorages[] = "deleteWebStorages";
const char kDetachGroup[] = "detachGroup";
const char kDisableBackButton[] = "disableBackButton";
const char kDisableInspectablePage[] = "disableInspectablePage";
const char kDisableWebSecurity[] = "disableWebSecurity";
const char kDoNotTrack[] = "doNotTrack";
const char kEnableInspectablePage[] = "enableInspectablePage";
const char kFlushCookies[] = "flushCookies";
const char kFocusGroupLayer[] = "focusGroupLayer";
const char kFocusGroupOwner[] = "focusGroupOwner";
const char kGetDevToolsEndpoint[] = "getDevToolsEndpoint";
const char kGetDocumentTitle[] = "getDocumentTitle";
const char kGetDocumentUrl[] = "getDocumentUrl";
const char kGetPageLifecycleState[] = "getPageLifecycleState";
const char kGetPid[] = "getPid";
const char kGetUserAgent[] = "getUserAgent";
const char kGetWindowState[] = "getWindowState";
const char kGetWindowStateAboutToChange[] = "getWindowStateAboutToChange";
const char kGoBack[] = "goBack";
const char kIgnoreSSLError[] = "ignoreSSLError";
const char kIsKeyboardVisible[] = "isKeyboardVisible";
const char kIsProfileCreated[] = "isProfileCreated";
const char kKillApp[] = "killApp";
const char kLaunchApp[] = "launchApp";
const char kLoadInjections[] = "loadInjections";
const char kMinimizeApp[] = "minimizeApp";
const char kNotifyMemoryPressure[] = "notifyMemoryPressure";
const char kReloadPage[] = "reloadPage";
const char kReplaceBaseURL[] = "replaceBaseURL";
const char kResetCompositorPainting[] = "resetCompositorPainting";
const char kResizeWindow[] = "resizeWindow";
const char kResumeDOM[] = "resumeDOM";
const char kResumeMedia[] = "resumeMedia";
const char kResumePainting[] = "resumePainting";
const char kRunJavaScript[] = "runJavaScript";
const char kRunJSInAllFrames[] = "runJSInAllFrames";
const char kSetAcceptLanguages[] = "setAcceptLanguages";
const char kSetAllowFakeBoldText[] = "setAllowFakeBoldText";
const char kSetBackgroundColor[] = "setBackgroundColor";
const char kSetBoardType[] = "setBoardType";
const char kSetCustomCursor[] = "setCustomCursor";
const char kSetDisallowScrollbarsInMainFrame[] = "setDisallowScrollbarsInMainFrame";
const char kSetExtraWebSocketHeader[] = "setExtraWebSocketHeader";
const char kSetFocus[] = "setFocus";
const char kSetFontFamily[] = "setFontFamily";
const char kSetGroupKeyMask[] = "setGroupKeyMask";
const char kSetHTMLSystemKeyboardEnabled[] = "setHTMLSystemKeyboardEnabled";
const char kSetInputRegion[] = "setInputRegion";
const char kSetInspectable[] = "setInspectable";
const char kSetKeyMask[] = "setKeyMask";
const char kSetMediaCapturePermission[] = "setMediaCapturePermission";
const char kSetMediaCodecCapability[] = "setMediaCodecCapability";
const char kSetOpacity[] = "setOpacity";
const char kSetProfile[] = "setProfile";
const char kSetProxyServer[] = "setProxyServer";
const char kSetUseVirtualKeyboard[] = "setUseVirtualKeyboard";
const char kSetUserAgent[] = "setUserAgent";
const char kSetVisibilityState[] = "setVisibilityState";
const char kSetWindowProperty[] = "setWindowProperty";
const char kSetWindowState[] = "setWindowState";
const char kShowApp[] = "showApp";
const char kSimulateNetworkState[] = "simulateNetworkState";
const char kStopApp[] = "stopApp";
const char kStopLoading[] = "stopLoading";
const char kSuspendDOM[] = "suspendDOM";
const char kSuspendMedia[] = "suspendMedia";
const char kSuspendPainting[] = "suspendPainting";
const char kUpdateAppWindow[] = "updateAppWindow";
const char kUpdateZoom[] = "updateZoom";
const char kXInputActivate[] = "xinputActivate";
const char kXInputDeactivate[] = "xinputDeactivate";
const char kXInputInvokeAction[] = "xinputInvokeAction";
const char kEnableUnlimitedMediaPolicy[] = "enableUnlimitedMediaPolicy";
const char kDisableUnlimitedMediaPolicy[] = "disableUnlimitedMediaPolicy";

}  // namespace command

namespace response {

const char kAppClosed[] = "appClosed";
const char kAppStarted[] = "appStarted";
const char kDevToolsEndpoint[] = "devToolsEndpoint";
const char kDocumentTitle[] = "documentTitle";
const char kDocumentUrl[] = "documentUrl";
const char kCanGoBackAbility[] = "canGoBackAbility";
const char kCursorUpdated[] = "cursorUpdated";
const char kFirstContentfulPaintTime[] = "firstContentfulPaintTime";
const char kFirstImagePaintTime[] = "firstImagePaintTime";
const char kFirstMeaningfulPaintTime[] = "firstMeaningfulPaintTime";
const char kFirstPaintTime[] = "firstPaintTime";
const char kKeyboardVisibility[] = "keyboardVisibility";
const char kLargestContentfulPaintTime[] = "largestContentfulPaintTime";
const char kLoadingEndTime[] = "loadingEndTime";
const char kLoadFailed[] = "loadFailed";
const char kLoadFinished[] = "loadFinished";
const char kPageLifecycleState[] = "pageLifecycleState";
const char kPidRequested[] = "pidRequested";
const char kPidUpdated[] = "pidUpdated";
const char kProcessGone[] = "processGone";
const char kProfileCreated[] = "profileCreated";
const char kUserAgentIs[] = "userAgentIs";
const char kWindowStateAboutToChangeRequested[] = "windowStateAboutToChangeRequested";
const char kWindowStateRequested[] = "windowStateRequested";
const char kZoomUpdated[] = "zoomUpdated";

}  // namespace response

namespace argument {

const char kAcceptLanguages[] = "accept_languages";
const char kAllow[] = "allow";
const char kAppId[] = "app_id";
const char kBlueColor[] = "blue";
const char kBoardType[] = "board_type";
const char kBrowsingDataMask[] = "browsing_data_mask";
const char kCmd[] = "cmd";
const char kConnected[] = "connected";
const char kDisable[] = "disable";
const char kDisallow[] = "disallow";
const char kEnable[] = "enable";
const char kFontFamily[] = "font_family";
const char kFramelessWindow[] = "frameless_window";
const char kFullScreen[] = "full_screen";
const char kGreenColor[] = "green";
const char kGroupName[] = "group_name";
const char kHeaderName[] = "header_name";
const char kHeaderValue[] = "header_value";
const char kHeight[] = "height";
const char kInputRegion[] = "input_region";
const char kIsOwner[] = "is_owner";
const char kJSCode[] = "javascript_code";
const char kKeyMask[] = "key_mask";
const char kKeySym[] = "key_sym";
const char kMediaCodecCapability[] = "media_codec_capability";
const char kMemoryPressureLevel[] = "memory_pressure_level";
const char kName[] = "name";
const char kOpacity[] = "opacity";
const char kPosX[] = "pos_x";
const char kPosY[] = "pos_y";
const char kProfile[] = "profile";
const char kProxyBypassList[] = "proxy_bypass_list";
const char kProxyEnabled[] = "proxy_enabled";
const char kProxyLogin[] = "proxy_login";
const char kProxyPassword[] = "proxy_password";
const char kProxyPort[] = "proxy_port";
const char kProxyServer[] = "proxy_server";
const char kRedColor[] = "red";
const char kResolutionHeight[] = "resolution_height";
const char kResolutionWidth[] = "resolution_width";
const char kSet[] = "set";
const char kTransparentBackground[] = "transparent_background";
const char kUrl[] = "url";
const char kUserAgent[] = "user_agent";
const char kUserStyleSheet[] = "user_stylesheet";
const char kValue[] = "value";
const char kViewportHeight[] = "viewport_height";
const char kViewportWidth[] = "viewport_width";
const char kVisibilityState[] = "visibility_state";
const char kWidth[] = "width";
const char kWindowHeight[] = "window_height";
const char kWindowState[] = "window_state";
const char kWindowWidth[] = "window_width";
const char kZoomFactor[] = "zoom";

const int kDefaultAlphaValue = 255;

}  // namespace argument
}  // namespace wam_demo
