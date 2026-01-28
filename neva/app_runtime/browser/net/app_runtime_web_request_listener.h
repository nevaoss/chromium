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
//
// This file is based on shell/browser/net/web_request_api_interface.h
// from ElectronJS project.
//
// Copyright (c) 2020 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#ifndef NEVA_APP_RUNTIME_BROWSER_NET_APP_RUNTIME_WEB_REQUEST_LISTENER_H_
#define NEVA_APP_RUNTIME_BROWSER_NET_APP_RUNTIME_WEB_REQUEST_LISTENER_H_

#include <set>
#include <string>

#include "extensions/browser/api/web_request/web_request_info.h"
#include "net/base/completion_once_callback.h"
#include "services/network/public/cpp/resource_request.h"

namespace neva_app_runtime {

class AppRuntimeWebRequestListener {
 public:
  virtual ~AppRuntimeWebRequestListener() {}

  using BeforeSendHeadersCallback =
      base::OnceCallback<void(const std::set<std::string>& removed_headers,
                              const std::set<std::string>& set_headers,
                              int error_code)>;

  virtual bool HasHandlers() const = 0;
  virtual int OnBeforeRequest(extensions::WebRequestInfo* info,
                              const network::ResourceRequest& request,
                              net::CompletionOnceCallback callback,
                              GURL* new_url) = 0;
  virtual int OnBeforeSendHeaders(extensions::WebRequestInfo* info,
                                  const network::ResourceRequest& request,
                                  BeforeSendHeadersCallback callback,
                                  net::HttpRequestHeaders* headers) = 0;
  virtual int OnHeadersReceived(
      extensions::WebRequestInfo* info,
      const network::ResourceRequest& request,
      net::CompletionOnceCallback callback,
      const net::HttpResponseHeaders* original_response_headers,
      scoped_refptr<net::HttpResponseHeaders>* override_response_headers,
      GURL* allowed_unsafe_redirect_url) = 0;
  virtual void OnSendHeaders(extensions::WebRequestInfo* info,
                             const network::ResourceRequest& request,
                             const net::HttpRequestHeaders& headers) = 0;
  virtual void OnBeforeRedirect(extensions::WebRequestInfo* info,
                                const network::ResourceRequest& request,
                                const GURL& new_location) = 0;
  virtual void OnResponseStarted(extensions::WebRequestInfo* info,
                                 const network::ResourceRequest& request) = 0;
  virtual void OnErrorOccurred(extensions::WebRequestInfo* info,
                               const network::ResourceRequest& request,
                               int net_error) = 0;
  virtual void OnCompleted(extensions::WebRequestInfo* info,
                           const network::ResourceRequest& request,
                           int net_error) = 0;
  virtual void OnRequestWillBeDestroyed(extensions::WebRequestInfo* info) = 0;
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_BROWSER_NET_APP_RUNTIME_WEB_REQUEST_DELEGATE_H_
