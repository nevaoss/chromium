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

#include "neva/browser_shell/service/browser_shell_webrequest_impl.h"

#include <optional>

#include "base/functional/bind.h"
#include "base/json/json_reader.h"
#include "base/logging.h"
#include "base/notimplemented.h"
#include "base/values.h"
#include "extensions/common/url_pattern.h"
#include "neva/app_runtime/browser/app_runtime_browser_context.h"
#include "neva/app_runtime/browser/app_runtime_storage_partition_spec.h"

namespace browser_shell {

namespace {

std::string ConvertWebRequestResourceTypeToString(
    extensions::WebRequestResourceType type) {
  switch (type) {
    case extensions::WebRequestResourceType::MAIN_FRAME:
      return "mainFrame";
    case extensions::WebRequestResourceType::SUB_FRAME:
      return "subFrame";
    case extensions::WebRequestResourceType::STYLESHEET:
      return "stylesheet";
    case extensions::WebRequestResourceType::SCRIPT:
      return "script";
    case extensions::WebRequestResourceType::IMAGE:
      return "image";
    case extensions::WebRequestResourceType::FONT:
      return "font";
    case extensions::WebRequestResourceType::OBJECT:
      return "object";
    case extensions::WebRequestResourceType::XHR:
      return "xhr";
    case extensions::WebRequestResourceType::PING:
      return "ping";
    case extensions::WebRequestResourceType::CSP_REPORT:
      return "cspReport";
    case extensions::WebRequestResourceType::MEDIA:
      return "media";
    case extensions::WebRequestResourceType::WEB_SOCKET:
      return "webSocket";
    default:
      return "other";
  }
}

void FillDetailsWith(browser_shell::mojom::DetailsPtr& details,
                     const extensions::WebRequestInfo* info) {
  details->id = info->id;
  details->url = info->url.spec();
  details->method = info->method;
  details->timestamp = base::Time::Now().InSecondsFSinceUnixEpoch() * 1000;
  details->resource_type =
      ConvertWebRequestResourceTypeToString(info->web_request_type);
  if (!info->response_ip.empty())
    details->response_ip = info->response_ip;
  details->from_cache = info->response_from_cache;
}

void FillDetailsWith(browser_shell::mojom::DetailsPtr& details,
                     const network::ResourceRequest& resource_request) {
  NOTIMPLEMENTED();
}

}  // namespace

WebRequestImpl::WebRequestImpl(const std::string& spec) {
  auto* context = neva_app_runtime::AppRuntimeBrowserContext::From(
      neva_app_runtime::ParseStoragePartitionSpec(spec));
  web_request_ = neva_app_runtime::AppRuntimeWebRequestHandler::From(context);
}

WebRequestImpl::~WebRequestImpl() = default;

void WebRequestImpl::OnBeforeRequest(
    const extensions::WebRequestInfo* info,
    const network::ResourceRequest& resource_request,
    OnBeforeRequestHandlerCallback callback) {
  auto details = mojom::Details::New();
  FillDetailsWith(details, info);
  FillDetailsWith(details, resource_request);

  // Here we have a little hack. OnBeforeRequestHandlerCallback and
  // mojom::OnBeforeRequestCallback strongly speaking are not the same.
  // But... they have the same signature.
  remote_client_->OnBeforeRequest(std::move(details), std::move(callback));
}

void WebRequestImpl::BindClient(BindClientCallback callback) {
  std::move(callback).Run(remote_client_.BindNewEndpointAndPassReceiver());
}

void WebRequestImpl::SetOnBeforeRequestListener(const std::string& json) {
  std::set<URLPattern> patterns;

  std::optional<base::Value> json_val =
      base::JSONReader::Read(json, base::JSON_PARSE_CHROMIUM_EXTENSIONS);
  if (json_val && json_val->is_dict()) {
    base::ListValue* urls_list = json_val->GetDict().FindList("urls");
    if (urls_list) {
      for (base::Value& val : *urls_list) {
        std::string* filter = val.GetIfString();
        if (!filter)
          continue;
        URLPattern pattern(URLPattern::SCHEME_ALL);
        URLPattern::ParseResult result = pattern.Parse(*filter);
        if (result == URLPattern::ParseResult::kSuccess)
          patterns.insert(pattern);
        else
          LOG(ERROR) << "Invalid url pattern " << *filter;
      }
    }
  }

  web_request_->SetOnBeforeRequestHandler(
      std::move(patterns), base::BindRepeating(&WebRequestImpl::OnBeforeRequest,
                                               weak_factory_.GetWeakPtr()));
}

void WebRequestImpl::ResetOnBeforeRequestListener() {
  web_request_->SetOnBeforeRequestHandler(
      std::set<URLPattern>(),
      neva_app_runtime::AppRuntimeWebRequestHandler::OnBeforeRequestHandler());
}

}  // namespace browser_shell
