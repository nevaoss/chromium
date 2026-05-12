// Copyright 2022 LG Electronics, Inc.
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

#ifndef NEVA_INJECTION_RENDERER_BROWSER_SHELL_BROWSER_SHELL_WEBREQUEST_H_
#define NEVA_INJECTION_RENDERER_BROWSER_SHELL_BROWSER_SHELL_WEBREQUEST_H_

#include <string>

#include "gin/object_template_builder.h"
#include "gin/wrappable.h"
#include "mojo/public/cpp/bindings/associated_receiver.h"
#include "mojo/public/cpp/bindings/pending_associated_receiver.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "neva/browser_shell/service/public/mojom/browser_shell_webrequest.mojom.h"
#include "v8/include/v8.h"

namespace injections {

class BrowserShellWebRequest : public gin::Wrappable<BrowserShellWebRequest>,
                               public browser_shell::mojom::WebRequestClient {
 public:
  static gin::WrapperInfo kWrapperInfo;

  BrowserShellWebRequest(
      v8::Isolate* isolate,
      mojo::Remote<browser_shell::mojom::WebRequest> remote);
  BrowserShellWebRequest(const BrowserShellWebRequest&) = delete;
  BrowserShellWebRequest& operator=(const BrowserShellWebRequest&) = delete;
  ~BrowserShellWebRequest() override;

  void SetupClient(
      mojo::PendingAssociatedReceiver<browser_shell::mojom::WebRequestClient>
          receiver);

  void SetOnBeforeRequestListener(gin::Arguments* args);

  // browser_shell::mojom::WebRequestClient:
  void OnBeforeRequest(browser_shell::mojom::DetailsPtr details,
                       OnBeforeRequestCallback callback) override;
 private:
  void HandleOnBeforeRequest(browser_shell::mojom::DetailsPtr& details,
                             bool* cancel,
                             std::string* redirect_url);

  v8::Global<v8::Function> on_before_request_listener_;

  mojo::Remote<browser_shell::mojom::WebRequest> remote_;
  mojo::AssociatedReceiver<browser_shell::mojom::WebRequestClient>
      client_receiver_;

  // gin::Wrappable.
  gin::ObjectTemplateBuilder GetObjectTemplateBuilder(
      v8::Isolate* isolate) final;
};

}  // namespace injections

#endif  // NEVA_INJECTION_RENDERER_BROWSER_SHELL_BROWSER_SHELL_WEBREQUEST_H_
