// Copyright 2019 LG Electronics, Inc.
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

#include "neva/pal_service/webos/system_servicebridge_delegate_webos.h"

#include "base/functional/callback.h"
#include "neva/pal_service/luna/luna_names.h"

namespace {

pal::mojom::ResponseStatus ConvertLunaClientStatusToPalStatus(
    pal::luna::Client::ResponseStatus status) {
  switch (status) {
    case pal::luna::Client::ResponseStatus::SUCCESS:
      return pal::mojom::ResponseStatus::kSuccess;
    case pal::luna::Client::ResponseStatus::CANCELED:
      return pal::mojom::ResponseStatus::kCanceled;
    case pal::luna::Client::ResponseStatus::ERROR:
      return pal::mojom::ResponseStatus::kError;
  }
  return pal::mojom::ResponseStatus::kError;
}

}  // anonymous namespace

namespace pal {
namespace webos {

SystemServiceBridgeDelegateWebOS::SystemServiceBridgeDelegateWebOS(
    CreationParams params,
    Response callback)
    : callback_(std::move(callback)), weak_factory_(this) {
  luna::Client::Params luna_params;
  if (params.name.empty()) {
    if (params.suffix == -1) {
      luna_params.name = luna::GetServiceNameWithPID(params.appid, "-");
    } else {
      luna_params.name = luna::GetServiceName(params.appid, params.suffix, "-");
    }
  } else {
    luna_params.name = std::move(params.name);
  }

  luna_params.appid = std::move(params.appid);
  luna_client_ = luna::GetSharedClient(luna_params);
}

SystemServiceBridgeDelegateWebOS::~SystemServiceBridgeDelegateWebOS() {
  Cancel();
}

void SystemServiceBridgeDelegateWebOS::Call(std::string uri,
                                            std::string payload) {
  if (!luna_client_)
    return;

  unsigned token;
  if (luna::IsSubscription(payload)) {
    const bool subscribed = luna_client_->Subscribe(
        uri,
        payload,
        base::BindRepeating(&SystemServiceBridgeDelegateWebOS::OnSubscription,
                            weak_factory_.GetWeakPtr()),
        std::string("{}"),
        &token);
    if (subscribed)
      subscription_tokens_.insert(token);
  } else {
    const bool called = luna_client_->Call(
        uri,
        payload,
        base::BindOnce(&SystemServiceBridgeDelegateWebOS::OnResponse,
                       weak_factory_.GetWeakPtr()),
        std::string("{}"),
        &token);
    if (called)
      response_tokens_.insert(token);
  }
}

void SystemServiceBridgeDelegateWebOS::Cancel() {
  if (!luna_client_)
    return;

  std::set<unsigned> cached_subscription_tokens;
  cached_subscription_tokens.swap(subscription_tokens_);
  for (unsigned token : cached_subscription_tokens)
    luna_client_->Unsubscribe(token);

  std::set<unsigned> cached_response_tokens;
  cached_response_tokens.swap(response_tokens_);
  for (unsigned token : cached_response_tokens)
    luna_client_->Cancel(token);
}

void SystemServiceBridgeDelegateWebOS::OnSubscription(
    luna::Client::ResponseStatus status,
    unsigned token,
    const std::string& json) {
  const pal::mojom::ResponseStatus pal_status =
      ConvertLunaClientStatusToPalStatus(status);
  if (status != luna::Client::ResponseStatus::SUCCESS)
    subscription_tokens_.erase(token);
  callback_.Run(pal_status, json);
}

void SystemServiceBridgeDelegateWebOS::OnResponse(
    luna::Client::ResponseStatus status,
    unsigned token,
    const std::string& json) {
  const pal::mojom::ResponseStatus pal_status =
      ConvertLunaClientStatusToPalStatus(status);
  response_tokens_.erase(token);
  callback_.Run(pal_status, json);
}

}  // namespace webos
}  // namespace pal
