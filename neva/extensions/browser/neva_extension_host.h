// Copyright 2023 LG Electronics, Inc.
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

#ifndef NEVA_EXTENSIONS_BROWSER_NEVA_EXTENSION_HOST_H_
#define NEVA_EXTENSIONS_BROWSER_NEVA_EXTENSION_HOST_H_

#include "extensions/browser/extension_host.h"

namespace neva {

class NevaExtensionHost : public extensions::ExtensionHost {
 public:
  using SizeUpdateHandler =
      base::RepeatingCallback<void(ExtensionHost*, int, int)>;

  NevaExtensionHost(const extensions::Extension* extension,
                    content::SiteInstance* site_instance,
                    const GURL& url,
                    extensions::mojom::ViewType host_type);

  ~NevaExtensionHost() override;

  void ResizeDueToAutoResize(content::WebContents* web_contents,
                             const gfx::Size& new_size) override;

  void SetSizeUpdateHandler(SizeUpdateHandler size_update_handler);

 private:
  SizeUpdateHandler size_update_handler_;
};

}  // namespace neva

#endif  // NEVA_EXTENSIONS_BROWSER_NEVA_EXTENSION_HOST_H_
