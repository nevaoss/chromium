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

#ifndef NEVA_APP_RUNTIME_APP_APP_RUNTIME_PAGE_CONTENTS_DELEGATE_H_
#define NEVA_APP_RUNTIME_APP_APP_RUNTIME_PAGE_CONTENTS_DELEGATE_H_

#include <string>
#include <vector>

#include "third_party/blink/public/mojom/favicon/favicon_url.mojom.h"

namespace neva_app_runtime {

class PageContents;

struct AuthChallengeInfo {
  AuthChallengeInfo();
  AuthChallengeInfo(const AuthChallengeInfo&);
  AuthChallengeInfo& operator=(AuthChallengeInfo&);
  ~AuthChallengeInfo();

  std::string url;
  std::string host;
  bool is_proxy;
  int port;
  std::string realm;
  std::string scheme;
};

struct FaviconSize {
  unsigned int width = 0;
  unsigned int height = 0;
};

struct FaviconInfo {
  FaviconInfo();
  FaviconInfo(const FaviconInfo&);
  FaviconInfo& operator=(FaviconInfo&);
  ~FaviconInfo();

  std::string url;
  std::string type;
  std::vector<FaviconSize> sizes;
};

struct NewWindowInfo {
  NewWindowInfo();
  NewWindowInfo(const NewWindowInfo&);
  NewWindowInfo& operator=(NewWindowInfo&);
  ~NewWindowInfo();

  std::string target_url;
  int32_t initial_width;
  int32_t initial_height;
  std::string name;
  std::string window_open_disposition;
  bool user_gesture;
};

struct OpenURLInfo {
  OpenURLInfo();
  OpenURLInfo(const OpenURLInfo&);
  OpenURLInfo& operator=(OpenURLInfo&);
  ~OpenURLInfo();
  std::string target_url;
  std::string window_open_disposition;
  bool user_gesture;
};

std::string ContentIconTypeToString(blink::mojom::FaviconIconType icon_type);

class PageContentsDelegate {
 public:
  virtual ~PageContentsDelegate();

  virtual void ActivateContents();
  virtual void EnterHtmlFullscreen();
  virtual void DidFailLoad(const std::string& url,
                           bool is_main_frame,
                           const std::string& error,
                           int error_code);
  virtual void DidFinishLoad(const std::string& url, bool is_main_frame);
  virtual void DidFinishNavigation(const std::string& url);
  virtual void DidPushHistoryNavigation(const std::string& url);
  virtual void DidStartLoading();
  virtual void DidStartNavigation(const std::string& url);
  virtual void DidStopLoading();
  virtual void DidUpdateFaviconUrl(const std::vector<FaviconInfo>& info);
  virtual void DOMReady();
  virtual void LeaveHtmlFullscreen();
  virtual void LoadProgressChanged(uint32_t progress);
  virtual void NavigationEntryCommitted();
  virtual void OnAcceptedLanguagesChanged(const std::string& languages);
  virtual void OnAuthChallenge(AuthChallengeInfo& challenge);
  virtual void OnCapturingMediaStreamChanged(bool is_capturing_media_stream);
  virtual void OnClose();
  virtual void OnExit(const std::string& reason);
  virtual void OnFocusChanged(bool is_focused);
  virtual void OnKeyEvent(const std::string& key_name, int32_t key_code);
  virtual void OnMouseClickEvent(int button_code);
  virtual void OnNewWindowOpen(std::unique_ptr<PageContents> new_contents,
                               NewWindowInfo& window_info);
  virtual void OnOpenURLFromTab(OpenURLInfo& open_info);
  virtual void OnRendererUnresponsive();
  virtual void OnRendererResponsive();
  virtual void OnPermissionRequest(const std::string& permission, uint64_t id);
  virtual void OnTouchStartEvent();
  virtual void OnVisibleRegionCaptured(const std::string& base64_data);
  virtual void OnZoomFactorChanged(double zoom_factor);
  virtual bool RunJSDialog(const std::string& type, const std::string& message);
  virtual void TitleUpdated(const std::string& title);

  virtual void OnDestroying(PageContents* contents);
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_APP_APP_RUNTIME_PAGE_CONTENTS_DELEGATE_H_
