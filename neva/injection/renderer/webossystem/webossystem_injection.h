// Copyright 2014-2019 LG Electronics, Inc.
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

#ifndef NEVA_INJECTION_RENDERER_WEBOSSYSTEM_WEBOSSYSTEM_INJECTION_H_
#define NEVA_INJECTION_RENDERER_WEBOSSYSTEM_WEBOSSYSTEM_INJECTION_H_

#include "gin/object_template_builder.h"
#include "gin/wrappable.h"
#include "neva/injection/renderer/injection_browser_control_base.h"
#include "neva/injection/renderer/webossystem/cursor_injection.h"
#include "neva/injection/renderer/webossystem/webossystem_datamanager.h"
#include "neva/injection/renderer/webossystem/window_injection.h"
#include "v8/include/v8.h"

namespace blink {
class WebLocalFrame;
}

namespace gin {
class Arguments;
}

namespace injections {

class WebOSSystemInjection : public gin::Wrappable<WebOSSystemInjection>,
                             public InjectionBrowserControlBase,
                             public WindowInjection::Delegate,
                             public CursorInjection::Delegate {
 public:
  static gin::WrapperInfo kWrapperInfo;
  static void Install(blink::WebLocalFrame* frame);
  static void Uninstall(blink::WebLocalFrame* frame);

  explicit WebOSSystemInjection(blink::WebLocalFrame* frame);
  WebOSSystemInjection(const WebOSSystemInjection&) = delete;
  WebOSSystemInjection& operator=(const WebOSSystemInjection&) = delete;
  ~WebOSSystemInjection() override;

  void BuildExtraObjects(v8::Local<v8::Object> obj,
                         v8::Isolate* isolate,
                         v8::Local<v8::Context> context);

  // Override WindowInjection::Delegate
  void SetInputRegion(gin::Arguments* args) override;
  void SetWindowProperty(const std::string& arg1,
                         const std::string& arg2) override;
  std::string GetPropertyValue(const std::string& name);

  // Override CursorInjection::Delegate
  std::string CallFunctionName(const std::string& name) override;
  bool HideCursor() override;
  bool SetCursor(gin::Arguments* args) override;

 private:
  static v8::MaybeLocal<v8::Object> CreateWebOSSystemObject(
      blink::WebLocalFrame* frame,
      v8::Isolate* isolate,
      v8::Local<v8::Object> global);

  // gin::Wrappable.
  gin::ObjectTemplateBuilder GetObjectTemplateBuilder(
      v8::Isolate* isolate) final;

  // Handlers for JS properties.
  std::string GetResource(const std::string& arg);
  double DevicePixelRatio();
  std::string GetCountry();
  bool GetIsMinimal();
  std::string GetActivityId();
  std::string GetDeviceInfo();
  std::string GetTimeZone();
  std::string GetTimeFormat();
  std::string GetIdentifier();
  std::string GetHighContrast();
  std::string GetLocale();
  std::string GetLocaleRegion();
  std::string GetPhoneRegion();
  std::string GetScreenOrientation();
  bool GetIsActivated();
  std::string GetLaunchParams();
  void SetLaunchParams(const std::string& launch_params);
  std::string GetWindowOrientation();
  void SetWindowOrientation(const std::string& param);
  bool GetIsKeyboardVisible();
  void Activate();
  void Deactivate();
  void OnCloseNotify(const std::string& param);
  void NativePmLogInfoWithClock(gin::Arguments* args);
  void NativePmLogString(gin::Arguments* args);
  void NativePmTrace(const std::string& param);
  void NativePmTraceItem(const std::string& param0, const std::string& param1);
  void NativePmTraceBefore(const std::string& param);
  void NativePmTraceAfter(const std::string& param);
  void AddBannerMessage(gin::Arguments* args);
  void RemoveBannerMessage(const std::string& param);
  void ClearBannerMessages();
  void SimulateMouseClick(const std::string& result0,
                          const std::string& result1,
                          bool result2);
  void UseSimulatedMouseClicks(bool param);
  void Paste();
  void CopiedToClipboard();
  void PastedFromClipboard();
  void MarkFirstUseDone();
  void EnableFullScreenMode(bool param);
  void StagePreparing();
  void StageReady();
  void ContainerReady();
  void EditorFocused();
  void KeepAlive(bool param);
  void ApplyLaunchFeedback();
  std::string AddNewContentIndicator();
  void RemoveNewContentIndicator(const std::string& param);
  void KeyboardShow(int param);
  void KeyboardHide();
  void SetManualKeyboardEnabled(bool param);
  void PlatformBack();
  void SetKeyMask(const std::vector<std::string>& str_array);
  void SetLoadErrorPolicy(const std::string& param);
  void Hide();
  void FocusOwner();
  void FocusLayer();
  std::string ServiceCall(const std::string& uri, const std::string& payload);
  void SetAppInClosing();
  bool DidRunOnCloseCallback();
  void UpdateInjectionData(const std::string& key, const std::string& value);
  void ReloadInjectionData();
  bool SetCursorHidden(gin::Arguments* args);

  std::string GetInjectionData(const std::string& name);

  WebOSSystemDataManager* data_manager_;
};

}  // namespace injections

#endif  // NEVA_INJECTION_RENDERER_WEBOSSYSTEM_WEBOSSYSTEM_INJECTION_H_
