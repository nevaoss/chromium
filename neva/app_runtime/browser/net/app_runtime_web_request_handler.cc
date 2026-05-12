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

#include "neva/app_runtime/browser/net/app_runtime_web_request_handler.h"

#include "base/no_destructor.h"
#include "neva/app_runtime/browser/app_runtime_browser_context.h"

namespace neva_app_runtime {

namespace {

bool MatchesFilterCondition(const extensions::WebRequestInfo* info,
                            const std::set<URLPattern>& patterns) {
  if (patterns.empty())
    return true;

  for (const auto& pattern : patterns) {
    if (pattern.MatchesURL(info->url))
      return true;
  }

  return false;
}

}  // namespace

AppRuntimeWebRequestHandler* AppRuntimeWebRequestHandler::From(
    content::BrowserContext* browser_context) {
  std::unique_ptr<AppRuntimeWebRequestHandler>& handler =
      web_request_map()[browser_context];

  if (!handler.get()) {
    handler = std::unique_ptr<AppRuntimeWebRequestHandler>(
        new AppRuntimeWebRequestHandler(browser_context));
  }
  return handler.get();
}

AppRuntimeWebRequestHandler* AppRuntimeWebRequestHandler::From(
    const std::string& partition_name,
    bool partition_off_the_record) {
  content::BrowserContext* browser_context =
      AppRuntimeBrowserContext::From(partition_name, partition_off_the_record);
  return AppRuntimeWebRequestHandler::From(browser_context);
}

AppRuntimeWebRequestHandler::~AppRuntimeWebRequestHandler() = default;

content::BrowserContext* AppRuntimeWebRequestHandler::GetBrowserContext() const {
  return browser_context_;
}

void AppRuntimeWebRequestHandler::SetOnBeforeRequestHandler(
    std::set<URLPattern> url_patterns,
    OnBeforeRequestHandler handler) {
  on_before_request_url_patterns_ = std::move(url_patterns);
  on_before_request_handler_ = std::move(handler);
}

bool AppRuntimeWebRequestHandler::HasHandlers() const {
  return !on_before_request_handler_.is_null();
}

int AppRuntimeWebRequestHandler::OnBeforeRequest(
    extensions::WebRequestInfo* info,
    const network::ResourceRequest& request,
    net::CompletionOnceCallback callback,
    GURL* new_url) {
  if (on_before_request_handler_.is_null())
    return net::OK;

  if (!MatchesFilterCondition(info, on_before_request_url_patterns_))
    return net::OK;

  callbacks_[info->id] = std::move(callback);

  OnBeforeRequestHandlerCallback handler_callback =
      base::BindOnce(
          &AppRuntimeWebRequestHandler::OnBeforeRequestHandlerResult,
              base::Unretained(this), info->id, new_url);

  on_before_request_handler_.Run(info, request, std::move(handler_callback));
  return net::ERR_IO_PENDING;
}

int AppRuntimeWebRequestHandler::OnBeforeSendHeaders(
    extensions::WebRequestInfo* info,
    const network::ResourceRequest& request,
    BeforeSendHeadersCallback callback,
    net::HttpRequestHeaders* headers) {
  return  net::OK;
}

int AppRuntimeWebRequestHandler::OnHeadersReceived(
    extensions::WebRequestInfo* info,
    const network::ResourceRequest& request,
    net::CompletionOnceCallback callback,
    const net::HttpResponseHeaders* original_response_headers,
    scoped_refptr<net::HttpResponseHeaders>* override_response_headers,
    GURL* allowed_unsafe_redirect_url) {
  return  net::OK;
}

void AppRuntimeWebRequestHandler::OnErrorOccurred(
    extensions::WebRequestInfo* info,
    const network::ResourceRequest& request,
    int net_error) {
  callbacks_.erase(info->id);
}

void AppRuntimeWebRequestHandler::OnCompleted(
    extensions::WebRequestInfo* info,
    const network::ResourceRequest& request,
    int net_error) {
  callbacks_.erase(info->id);
}

void AppRuntimeWebRequestHandler::OnRequestWillBeDestroyed(
    extensions::WebRequestInfo* info) {
  callbacks_.erase(info->id);
}

AppRuntimeWebRequestHandler::AppRuntimeWebRequestHandler(
    content::BrowserContext* browser_context)
    : browser_context_(browser_context) {}

void AppRuntimeWebRequestHandler::OnBeforeRequestHandlerResult(
    uint64_t id,
    GURL* redirect_url,
    bool cancel,
    const std::string& url_result) {
  const auto it = callbacks_.find(id);
  if (it == std::end(callbacks_))
    return;

  int result = cancel ? net::ERR_BLOCKED_BY_CLIENT : net::OK;
  if (!url_result.empty())
    *redirect_url = GURL(url_result);
  base::SequencedTaskRunner::GetCurrentDefault()->PostTask(
      FROM_HERE, base::BindOnce(std::move(callbacks_[id]), result));
  callbacks_.erase(it);
}

// static
AppRuntimeWebRequestHandler::WebRequestMap&
AppRuntimeWebRequestHandler::web_request_map() {
  static base::NoDestructor<AppRuntimeWebRequestHandler::WebRequestMap>
      web_request_registry;
  return *web_request_registry;
}

}  // namespace browser_shell
