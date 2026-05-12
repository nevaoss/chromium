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

#ifndef NEVA_BROWSER_SHELL_SERVICE_BROWSER_SHELL_WEBREQUEST_IMPL_H_
#define NEVA_BROWSER_SHELL_SERVICE_BROWSER_SHELL_WEBREQUEST_IMPL_H_

#include <string>

#include "base/memory/raw_ptr.h"
#include "mojo/public/cpp/bindings/associated_remote.h"
#include "mojo/public/cpp/bindings/pending_associated_receiver.h"
#include "neva/app_runtime/app/app_runtime_shell_window_delegate.h"
#include "neva/app_runtime/browser/net/app_runtime_web_request_handler.h"
#include "neva/browser_shell/service/public/mojom/browser_shell_webrequest.mojom.h"

namespace browser_shell {

class WebRequestImpl : public mojom::WebRequest {
 public:
  WebRequestImpl(const std::string& spec);
  WebRequestImpl(const WebRequestImpl&) = delete;
  WebRequestImpl& operator=(const WebRequestImpl&) = delete;
  ~WebRequestImpl() override;

  using OnBeforeRequestHandlerCallback =
      neva_app_runtime::AppRuntimeWebRequestHandler::
          OnBeforeRequestHandlerCallback;

  void OnBeforeRequest(const extensions::WebRequestInfo* info,
                       const network::ResourceRequest& resource_request,
                       OnBeforeRequestHandlerCallback callback);

  // mojom::WebRequest
  void BindClient(BindClientCallback callback) override;
  void SetOnBeforeRequestListener(const std::string& json) override;
  void ResetOnBeforeRequestListener() override;

 private:
  raw_ptr<neva_app_runtime::AppRuntimeWebRequestHandler> web_request_;
  mojo::AssociatedRemote<mojom::WebRequestClient> remote_client_;
};

}  // namespace browser_shell

#endif  // NEVA_BROWSER_SHELL_SERVICE_BROWSER_SHELL_WEBREQUEST_IMPL_H_
