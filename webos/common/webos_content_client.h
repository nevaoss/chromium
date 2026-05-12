// Copyright 2016-2019 LG Electronics, Inc.
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

#ifndef WEBOS_COMMON_WEBOS_CONTENT_CLIENT_H_
#define WEBOS_COMMON_WEBOS_CONTENT_CLIENT_H_

#include "neva/app_runtime/common/app_runtime_content_client.h"

namespace webos {

class WebOSContentClient : public neva_app_runtime::AppRuntimeContentClient {
 public:
  ~WebOSContentClient() override {}

  // content::ContentClient implementation
  void AddPlugins(std::vector<content::ContentPluginInfo>* plugins) override;
};

}  // namespace webos

#endif  // WEBOS_COMMON_WEBOS_CONTENT_CLIENT_H_
