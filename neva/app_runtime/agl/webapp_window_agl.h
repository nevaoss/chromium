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

#ifndef NEVA_APP_RUNTIME_AGL_WEBAPP_WINDOW_AGL_H_
#define NEVA_APP_RUNTIME_AGL_WEBAPP_WINDOW_AGL_H_

#include <string>

namespace neva_app_runtime {

class WebAppWindow;

namespace agl {

// AGL wayland extension API for webOS/Lite
class WebAppWindowAgl {
 public:
  WebAppWindowAgl(WebAppWindow* webapp_window);
  WebAppWindowAgl(const WebAppWindowAgl&) = delete;
  WebAppWindowAgl& operator=(const WebAppWindowAgl&) = delete;
  ~WebAppWindowAgl() = default;

  void SetAglActivateApp(const std::string& app);
  void SetAglAppId(const std::string& title);
  void SetAglBackground();
  void SetAglReady();
  void SetAglPanel(uint32_t edge);

 private:
  WebAppWindow* webapp_window_;
};

}  // namespace agl
}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_AGL_WEBAPP_WINDOW_AGL_H_