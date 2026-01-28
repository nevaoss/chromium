// Copyright 2025 LG Electronics, Inc.
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

#ifndef NEVA_INJECTION_RENDERER_BROWSER_SHELL_BROWSER_SHELL_SITE_SETTINGS_H_
#define NEVA_INJECTION_RENDERER_BROWSER_SHELL_BROWSER_SHELL_SITE_SETTINGS_H_

#include <memory>
#include <string>
#include <vector>

#include "gin/object_template_builder.h"
#include "gin/persistent.h"
#include "gin/weak_cell.h"
#include "gin/wrappable.h"
#include "mojo/public/cpp/bindings/associated_receiver.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "neva/browser_shell/service/public/mojom/browser_shell_site_settings.mojom.h"
#include "v8/include/v8-cppgc.h"
#include "v8/include/v8.h"

namespace injections {

class BrowserShellSiteSettings final
    : public gin::Wrappable<BrowserShellSiteSettings>,
      public browser_shell::mojom::SiteSettingsClient {
 public:
  static constexpr gin::WrapperInfo kWrapperInfo = {
      {gin::kEmbedderNativeGin},
      gin::kBrowserShellSiteSettings};

  BrowserShellSiteSettings(
      v8::Isolate* isolate,
      mojo::Remote<browser_shell::mojom::SiteSettings> remote);
  BrowserShellSiteSettings(const BrowserShellSiteSettings&) = delete;
  BrowserShellSiteSettings& operator=(const BrowserShellSiteSettings&) = delete;
  ~BrowserShellSiteSettings() override;

  cppgc::Persistent<gin::WeakCell<BrowserShellSiteSettings>> GetAsWeak() {
    return gin::WrapPersistent(weak_factory_.GetWeakCell(
        v8::Isolate::GetCurrent()->GetCppHeap()->GetAllocationHandle()));
  }

  void GetAllSites(gin::Arguments* args);
  void GetSitesForSettingType(gin::Arguments* args);
  void GetOriginPermissions(gin::Arguments* args);
  void ResetOriginPermissions(gin::Arguments* args);

  // browser_shell::mojom::SiteSettingsClient
  void OnUpdate() override;

  void Trace(cppgc::Visitor* visitor) const final;

 private:
  // gin::WrappableBase
  const gin::WrapperInfo* wrapper_info() const override;

  void OnGetSitesReply(v8::Global<v8::Function> callback,
                       const std::vector<std::string>& sites);
  void OnGetOriginPermissionsReply(
      v8::Global<v8::Function> callback,
      std::vector<browser_shell::mojom::PermissionPtr> permissions);

  gin::ObjectTemplateBuilder GetObjectTemplateBuilder(
      v8::Isolate* isolate) final;

  mojo::Remote<browser_shell::mojom::SiteSettings> remote_;
  mojo::AssociatedReceiver<browser_shell::mojom::SiteSettingsClient>
      client_receiver_{this};
  gin::WeakCellFactory<BrowserShellSiteSettings> weak_factory_{this};
};

}  // namespace injections

#endif  // NEVA_INJECTION_RENDERER_BROWSER_SHELL_BROWSER_SHELL_SITE_SETTINGS_H_
