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

#ifndef NEVA_APP_RUNTIME_COMMON_APP_RUNTIME_CONTENT_CLIENT_H_
#define NEVA_APP_RUNTIME_COMMON_APP_RUNTIME_CONTENT_CLIENT_H_

#include "content/public/common/content_client.h"

namespace neva_app_runtime {

class AppRuntimeContentClient : public content::ContentClient {
 public:
  ~AppRuntimeContentClient() override;

  // content::ContentClient implementation
  std::string_view GetDataResource(
      int resource_id,
      ui::ResourceScaleFactor scale_factor) override;
  scoped_refptr<base::RefCountedMemory> GetDataResourceBytes(
      int resource_id) override;
  gfx::Image& GetNativeImageNamed(int resource_id) override;
  void AddPlugins(std::vector<content::WebPluginInfo>* plugins) override;
  void AddContentDecryptionModules(
      std::vector<content::CdmInfo>* cdms,
      std::vector<media::CdmHostFilePath>* cdm_host_file_paths) override;
  void ExposeInterfacesToBrowser(
      scoped_refptr<base::SequencedTaskRunner> io_task_runner,
      mojo::BinderMap* binders) override;
#if defined(USE_NEVA_CHROME_EXTENSIONS)
  void AddAdditionalSchemes(Schemes* schemes) override;
#endif  // defined(USE_NEVA_CHROME_EXTENSIONS)
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_COMMON_APP_RUNTIME_CONTENT_CLIENT_H_
