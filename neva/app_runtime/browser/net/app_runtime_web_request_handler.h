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

#ifndef NEVA_APP_RUNTIME_BROWSER_NET_APP_RUNTIME_WEB_REQUEST_HANDLER_H_
#define NEVA_APP_RUNTIME_BROWSER_NET_APP_RUNTIME_WEB_REQUEST_HANDLER_H_

#include <map>
#include <set>

#include "base/memory/raw_ptr.h"
#include "extensions/common/url_pattern.h"
#include "neva/app_runtime/browser/net/app_runtime_web_request_listener.h"

namespace content {
class BrowserContext;
}  // namepsace content

namespace neva_app_runtime {

class AppRuntimeWebRequestHandler : public AppRuntimeWebRequestListener {
 public:
  static AppRuntimeWebRequestHandler* From(
      content::BrowserContext* browser_context);
  static AppRuntimeWebRequestHandler* From(const std::string& partitionname,
                                           bool partition_off_the_record);

  ~AppRuntimeWebRequestHandler() override;
  content::BrowserContext* GetBrowserContext() const;

  using OnBeforeRequestHandlerCallback =
    base::OnceCallback<void(bool result, const std::string&)>;

  using OnBeforeRequestHandler =
    base::RepeatingCallback<void(const extensions::WebRequestInfo*,
                                 const network::ResourceRequest&,
                                 OnBeforeRequestHandlerCallback callback)>;

  void SetOnBeforeRequestHandler(std::set<URLPattern> url_pattern,
                                  OnBeforeRequestHandler handler);

  // AppRuntimeWebRequestListener:
  bool HasHandlers() const override;
  int OnBeforeRequest(extensions::WebRequestInfo* info,
                      const network::ResourceRequest& request,
                      net::CompletionOnceCallback callback,
                      GURL* new_url) override;
  int OnBeforeSendHeaders(extensions::WebRequestInfo* info,
                          const network::ResourceRequest& request,
                          BeforeSendHeadersCallback callback,
                          net::HttpRequestHeaders* headers) override;
  int OnHeadersReceived(
      extensions::WebRequestInfo* info,
      const network::ResourceRequest& request,
      net::CompletionOnceCallback callback,
      const net::HttpResponseHeaders* original_response_headers,
      scoped_refptr<net::HttpResponseHeaders>* override_response_headers,
      GURL* allowed_unsafe_redirect_url) override;
  void OnSendHeaders(extensions::WebRequestInfo* info,
                     const network::ResourceRequest& request,
                     const net::HttpRequestHeaders& headers) override {}
  void OnBeforeRedirect(extensions::WebRequestInfo* info,
                        const network::ResourceRequest& request,
                        const GURL& new_location) override {}
  void OnResponseStarted(extensions::WebRequestInfo* info,
                         const network::ResourceRequest& request) override {}
  void OnErrorOccurred(extensions::WebRequestInfo* info,
                       const network::ResourceRequest& request,
                       int net_error) override;
  void OnCompleted(extensions::WebRequestInfo* info,
                   const network::ResourceRequest& request,
                   int net_error) override;
  void OnRequestWillBeDestroyed(extensions::WebRequestInfo* info) override;

 private:
  AppRuntimeWebRequestHandler(content::BrowserContext* browser_context);

  void OnBeforeRequestHandlerResult(uint64_t id,
                                     GURL* redirect_url,
                                     bool cancel,
                                     const std::string& url_result);

  OnBeforeRequestHandler on_before_request_handler_;
  std::set<URLPattern> on_before_request_url_patterns_;

  std::map<uint64_t, net::CompletionOnceCallback> callbacks_;
  raw_ptr<content::BrowserContext> browser_context_;

  using WebRequestMap = std::map<
    content::BrowserContext*, std::unique_ptr<AppRuntimeWebRequestHandler>>;
  static WebRequestMap& web_request_map();
 };

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_BROWSER_NET_APP_RUNTIME_WEB_REQUEST_HANDLER_H_
