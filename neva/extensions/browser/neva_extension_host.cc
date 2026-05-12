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

#include "neva/extensions/browser/neva_extension_host.h"

namespace neva {

NevaExtensionHost::NevaExtensionHost(const extensions::Extension* extension,
                                     content::SiteInstance* site_instance,
                                     const GURL& url,
                                     extensions::mojom::ViewType host_type)
    : ExtensionHost(extension, site_instance, url, host_type) {}

NevaExtensionHost::~NevaExtensionHost() = default;

void NevaExtensionHost::ResizeDueToAutoResize(
    content::WebContents* web_contents,
    const gfx::Size& new_size) {
  VLOG(1) << __func__ << " new_size: " << new_size.ToString();
  if (!size_update_handler_.is_null()) {
    size_update_handler_.Run(this, new_size.width(), new_size.height());
  }
}

void NevaExtensionHost::SetSizeUpdateHandler(
    SizeUpdateHandler size_update_handler) {
  DCHECK(!size_update_handler);
  size_update_handler_ = std::move(size_update_handler);
}

}  // namespace neva
