// Copyright 2024 LG Electronics, Inc.
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

#ifndef NEVA_INJECTION_RENDERER_BROWSER_SHELL_BROWSER_SHELL_COOKIE_MAMAGER_H_
#define NEVA_INJECTION_RENDERER_BROWSER_SHELL_BROWSER_SHELL_COOKIE_MAMAGER_H_

#include <string>
#include <vector>

#include "gin/object_template_builder.h"
#include "gin/persistent.h"
#include "gin/weak_cell.h"
#include "gin/wrappable.h"
#include "mojo/public/cpp/bindings/associated_receiver.h"
#include "mojo/public/cpp/bindings/pending_associated_receiver.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "neva/browser_shell/service/public/mojom/browser_shell_cookie_manager.mojom.h"
#include "v8/include/v8-cppgc.h"
#include "v8/include/v8.h"

namespace injections {

class BrowserShellCookieManager final
    : public gin::Wrappable<BrowserShellCookieManager>,
      public browser_shell::mojom::CookieManagerClient {
 public:
  static constexpr gin::WrapperInfo kWrapperInfo = {
      {gin::kEmbedderNativeGin},
      gin::kBrowserShellCookieManager};

  BrowserShellCookieManager(
      v8::Isolate* isolate,
      mojo::Remote<browser_shell::mojom::CookieManager> remote);
  BrowserShellCookieManager(const BrowserShellCookieManager&) = delete;
  BrowserShellCookieManager& operator=(const BrowserShellCookieManager&) =
      delete;
  ~BrowserShellCookieManager() override;

  cppgc::Persistent<gin::WeakCell<BrowserShellCookieManager>> GetAsWeak() {
    return gin::WrapPersistent(weak_factory_.GetWeakCell(
        v8::Isolate::GetCurrent()->GetCppHeap()->GetAllocationHandle()));
  }

  void SetupClient(
      mojo::PendingAssociatedReceiver<browser_shell::mojom::CookieManagerClient>
          receiver);

  bool SetCookieOption(gin::Arguments* args);
  bool ClearAllCookies();
  bool GetAllCookiesForTesting(gin::Arguments* args);

  // gin::Wrappable : v8::Object::Wrappable
  void Trace(cppgc::Visitor* visitor) const final;

 private:
  // gin::WrappableBase
  const gin::WrapperInfo* wrapper_info() const override;

  void OnGetAllCookiesResponse(v8::Global<v8::Function> callback,
                               const std::vector<std::string>& url_list);

  mojo::Remote<browser_shell::mojom::CookieManager> remote_;
  mojo::AssociatedReceiver<browser_shell::mojom::CookieManagerClient>
      client_receiver_{this};

  // gin::Wrappable.
  gin::ObjectTemplateBuilder GetObjectTemplateBuilder(
      v8::Isolate* isolate) final;

  gin::WeakCellFactory<BrowserShellCookieManager> weak_factory_{this};
};

}  // namespace injections

#endif  // NEVA_INJECTION_RENDERER_BROWSER_SHELL_BROWSER_SHELL_COOKIE_MAMAGER_H_
