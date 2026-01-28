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

#ifndef WEBOS_AGL_WEBAPP_WINDOW_BASE_AGL_H_
#define WEBOS_AGL_WEBAPP_WINDOW_BASE_AGL_H_

#include <string>

#include "webos/common/webos_export.h"

namespace webos {

class WebAppWindowBase;

namespace agl {

// AGL wayland extension API for webOS/Lite
class WEBOS_EXPORT WebAppWindowBaseAgl {
 public:
  WebAppWindowBaseAgl(WebAppWindowBase* webapp_window_base);
  WebAppWindowBaseAgl(const WebAppWindowBaseAgl&) = delete;
  WebAppWindowBaseAgl& operator=(const WebAppWindowBaseAgl&) = delete;
  ~WebAppWindowBaseAgl() = default;

  void InitWindow();

  void SetAglActivateApp(const char* app_id);
  void SetAglAppId(const char* app_id);
  void SetAglBackground();
  void SetAglReady();
  void SetAglPanel(int edge);
  void SetWindowSurfaceId(int surface_id);

 private:
  WebAppWindowBase* webapp_window_base_;

  std::string app_id_;
  int pending_agl_edge_ = -1;
  bool pending_agl_background_ = false;
  bool pending_agl_ready_ = false;
};

}  // namespace agl
}  // namespace webos

#endif  // WEBOS_AGL_WEBAPP_WINDOW_BASE_AGL_H_