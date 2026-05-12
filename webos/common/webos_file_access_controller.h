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

#ifndef WEBOS_COMMON_WEBOS_FILE_ACCESS_CONTROLLER_H_
#define WEBOS_COMMON_WEBOS_FILE_ACCESS_CONTROLLER_H_

#include <string>
#include <vector>

#include "neva/app_runtime/common/app_runtime_file_access_controller.h"

namespace neva_app_runtime {
struct WebViewInfo;
}

namespace webos {

class WebOSFileAccessController
    : public neva_app_runtime::AppRuntimeFileAccessController {
 public:
  WebOSFileAccessController();

  // neva_app_runtime::AppRuntimeFileAccessController implementation
  bool IsAccessAllowed(
      const base::FilePath& path,
      const neva_app_runtime::WebViewInfo& webview_info) const override;

 private:
  void ParsePathsFromSettings(std::vector<std::string>& paths,
                              std::istringstream& stream) const;

  std::vector<std::string> allowed_target_paths_;
  std::vector<std::string> allowed_trusted_target_paths_;
  bool allow_all_access_ = true;
};

}  // namespace webos

#endif  // WEBOS_COMMON_WEBOS_FILE_ACCESS_CONTROLLER_H_
