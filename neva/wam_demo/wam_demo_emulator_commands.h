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

#ifndef NEVA_WAM_DEMO_WAM_DEMO_EMULATOR_COMMANDS_H_
#define NEVA_WAM_DEMO_WAM_DEMO_EMULATOR_COMMANDS_H_

namespace wam_demo {

namespace command {

extern const char kAddUserStyleSheet[];
extern const char kAllowLocalResourceLoad[];
extern const char kAllowUniversalAccessFromFileUrls[];
extern const char kAllowXFrameSameOrigin[];
extern const char kCanGoBack[];
extern const char kChangeUrl[];
extern const char kClearBrowsingData[];
extern const char kClearInjections[];
extern const char kClearMediaCapturePermission[];
extern const char kDecidePolicyForResponse[];
extern const char kDeleteWebStorages[];
extern const char kDetachGroup[];
extern const char kDisableBackButton[];
extern const char kDisableInspectablePage[];
extern const char kDisableWebSecurity[];
extern const char kDoNotTrack[];
extern const char kEnableInspectablePage[];
extern const char kFlushCookies[];
extern const char kFocusGroupLayer[];
extern const char kFocusGroupOwner[];
extern const char kGetDevToolsEndpoint[];
extern const char kGetDocumentTitle[];
extern const char kGetDocumentUrl[];
extern const char kGetPageLifecycleState[];
extern const char kGetPid[];
extern const char kGetUserAgent[];
extern const char kGetWindowState[];
extern const char kGetWindowStateAboutToChange[];
extern const char kGoBack[];
extern const char kIgnoreSSLError[];
extern const char kIsKeyboardVisible[];
extern const char kIsProfileCreated[];
extern const char kKillApp[];
extern const char kLaunchApp[];
extern const char kLoadInjections[];
extern const char kNotifyMemoryPressure[];
extern const char kReloadPage[];
extern const char kReplaceBaseURL[];
extern const char kResetCompositorPainting[];
extern const char kResizeWindow[];
extern const char kResumeDOM[];
extern const char kResumeMedia[];
extern const char kResumePainting[];
extern const char kRunJavaScript[];
extern const char kRunJSInAllFrames[];
extern const char kSetAcceptLanguages[];
extern const char kSetAllowFakeBoldText[];
extern const char kSetBackgroundColor[];
extern const char kSetBoardType[];
extern const char kSetCustomCursor[];
extern const char kSetDisallowScrollbarsInMainFrame[];
extern const char kSetExtraWebSocketHeader[];
extern const char kSetFocus[];
extern const char kSetFontFamily[];
extern const char kSetGroupKeyMask[];
extern const char kSetHTMLSystemKeyboardEnabled[];
extern const char kSetInputRegion[];
extern const char kSetInspectable[];
extern const char kSetKeyMask[];
extern const char kSetMediaCapturePermission[];
extern const char kSetMediaCodecCapability[];
extern const char kSetOpacity[];
extern const char kSetProfile[];
extern const char kSetProxyServer[];
extern const char kSetUseVirtualKeyboard[];
extern const char kSetUserAgent[];
extern const char kSetVisibilityState[];
extern const char kSetWindowProperty[];
extern const char kSetWindowState[];
extern const char kShowApp[];
extern const char kMinimizeApp[];
extern const char kSimulateNetworkState[];
extern const char kStopApp[];
extern const char kStopLoading[];
extern const char kSuspendDOM[];
extern const char kSuspendMedia[];
extern const char kSuspendPainting[];
extern const char kUpdateAppWindow[];
extern const char kUpdateZoom[];
extern const char kXInputActivate[];
extern const char kXInputDeactivate[];
extern const char kXInputInvokeAction[];
extern const char kEnableUnlimitedMediaPolicy[];
extern const char kDisableUnlimitedMediaPolicy[];

}  // namespace command

namespace response {

extern const char kAppClosed[];
extern const char kAppStarted[];
extern const char kDevToolsEndpoint[];
extern const char kDocumentTitle[];
extern const char kDocumentUrl[];
extern const char kCanGoBackAbility[];
extern const char kCursorUpdated[];
extern const char kFirstContentfulPaintTime[];
extern const char kFirstImagePaintTime[];
extern const char kFirstMeaningfulPaintTime[];
extern const char kFirstPaintTime[];
extern const char kKeyboardVisibility[];
extern const char kLargestContentfulPaintTime[];
extern const char kLoadingEndTime[];
extern const char kLoadFailed[];
extern const char kLoadFinished[];
extern const char kPageLifecycleState[];
extern const char kPidRequested[];
extern const char kPidUpdated[];
extern const char kProcessGone[];
extern const char kProfileCreated[];
extern const char kUserAgentIs[];
extern const char kWindowStateAboutToChangeRequested[];
extern const char kWindowStateRequested[];
extern const char kZoomUpdated[];

}  // namespace response

namespace argument {

extern const char kAcceptLanguages[];
extern const char kAllow[];
extern const char kAppId[];
extern const char kBlueColor[];
extern const char kBoardType[];
extern const char kBrowsingDataMask[];
extern const char kCmd[];
extern const char kConnected[];
extern const char kDisable[];
extern const char kDisallow[];
extern const char kEnable[];
extern const char kFontFamily[];
extern const char kFramelessWindow[];
extern const char kFullScreen[];
extern const char kGreenColor[];
extern const char kGroupName[];
extern const char kHeaderName[];
extern const char kHeaderValue[];
extern const char kHeight[];
extern const char kInputRegion[];
extern const char kIsOwner[];
extern const char kJSCode[];
extern const char kKeyMask[];
extern const char kKeySym[];
extern const char kMediaCodecCapability[];
extern const char kMemoryPressureLevel[];
extern const char kName[];
extern const char kNetworkQuietTimeout[];
extern const char kOpacity[];
extern const char kPosX[];
extern const char kPosY[];
extern const char kProfile[];
extern const char kProxyBypassList[];
extern const char kProxyEnabled[];
extern const char kProxyLogin[];
extern const char kProxyPassword[];
extern const char kProxyPort[];
extern const char kProxyServer[];
extern const char kRedColor[];
extern const char kResolutionHeight[];
extern const char kResolutionWidth[];
extern const char kSet[];
extern const char kTransparentBackground[];
extern const char kUrl[];
extern const char kUserAgent[];
extern const char kUserStyleSheet[];
extern const char kValue[];
extern const char kViewportHeight[];
extern const char kViewportWidth[];
extern const char kVisibilityState[];
extern const char kWidth[];
extern const char kWindowHeight[];
extern const char kWindowState[];
extern const char kWindowWidth[];
extern const char kZoomFactor[];

extern const int kDefaultAlphaValue;

}  // namespace argument

}  // namespace wam_demo

#endif  // NEVA_WAM_DEMO_WAM_DEMO_EMULATOR_COMMANDS_H_
