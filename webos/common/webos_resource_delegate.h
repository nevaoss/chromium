// Copyright 2017-2018 LG Electronics, Inc.
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

#ifndef WEBOS_COMMON_WEBOS_RESOURCE_DELEGATE_H_
#define WEBOS_COMMON_WEBOS_RESOURCE_DELEGATE_H_

#include "ui/base/resource/resource_bundle.h"

namespace webos {

class WebosResourceDelegate : public ui::ResourceBundle::Delegate {
 public:
  static void InitializeResourceBundle();

  WebosResourceDelegate() {}
  WebosResourceDelegate(const WebosResourceDelegate&) = delete;
  WebosResourceDelegate& operator=(const WebosResourceDelegate&) = delete;
  ~WebosResourceDelegate() override {}

  // ui:ResourceBundle::Delegate implementation:
  base::FilePath GetPathForResourcePack(
      const base::FilePath& pack_path,
      ui::ResourceScaleFactor scale_factor) override;
  gfx::Image GetImageNamed(int resource_id) override;
  gfx::Image GetNativeImageNamed(int resource_id) override;
  scoped_refptr<base::RefCountedMemory> LoadDataResourceBytes(
      int resource_id,
      ui::ResourceScaleFactor scale_factor) override;
  bool GetRawDataResource(int resource_id,
                          ui::ResourceScaleFactor scale_factor,
                          std::string_view* value) const override;
  bool GetLocalizedString(int message_id,
                          std::u16string* value) const override;

  bool LoadBrowserResources();
};

}  // namespace webos

#endif  // WEBOS_COMMON_WEBOS_RESOURCE_DELEGATE_H_
