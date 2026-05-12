// Copyright 2021 LG Electronics, Inc.
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

#include "neva/pal_service/webos/memorymanager_delegate_ose.h"

#include "base/json/json_reader.h"
#include "neva/pal_service/luna/luna_names.h"

namespace pal {
namespace webos {

namespace {
const char kSubscribeToLevelChanged[] =
    R"JSON({"category":"/com/webos/service/memorymanager", "method":"levelChanged"})JSON";
const char kGetMemoryStatusMethod[] = "getMemoryStatus";
}  // namespace

// static
std::unique_ptr<MemoryManagerDelegateWebOS>
MemoryManagerDelegateWebOS::Create() {
  return std::make_unique<MemoryManagerDelegateOSE>();
}

MemoryManagerDelegateOSE::MemoryManagerDelegateOSE() : weak_factory_(this) {
  weak_ptr_this_ = weak_factory_.GetWeakPtr();
}

MemoryManagerDelegateOSE::~MemoryManagerDelegateOSE() = default;

void MemoryManagerDelegateOSE::GetMemoryStatus(OnceResponse callback) {
  luna_client_->Call(
      luna::GetServiceURI(luna::service_uri::kServiceMemoryManager,
                          kGetMemoryStatusMethod),
      std::string(kGetMemoryStatusRequest),
      base::BindOnce(&MemoryManagerDelegateOSE::OnMemoryStatus, weak_ptr_this_,
                     std::move(callback)),
      std::string(kDefaultResponse));
}

void MemoryManagerDelegateOSE::SubscribeToLevelChanged(
    RepeatingResponse callback) {
  subscription_callback_ = std::move(callback);
  if (!subscribed_ && luna_client_->IsInitialized()) {
    subscription_token_ = luna_client_->Subscribe(
        luna::GetServiceURI(luna::service_uri::kPalmBus, kSignalAddMatch),
        std::string(kSubscribeToLevelChanged),
        base::BindRepeating(&MemoryManagerDelegateOSE::OnLevelChanged,
                            weak_ptr_this_));
    subscribed_ = true;
  }
}

void MemoryManagerDelegateOSE::OnMemoryStatus(OnceResponse callback,
                                              luna::Client::ResponseStatus,
                                              unsigned,
                                              const std::string& json) {
  std::optional<base::Value> root(base::JSONReader::Read(json));
  if (!root || !root->is_dict())
    return;

  // webOS OSE com.webos.service.memorymanager
  const base::Value::Dict& dict = root->GetDict();
  const std::string* status = dict.FindStringByDottedPath("system.level");

  if (status && !status->empty()) {
    const std::string response = ConvertToMemoryLevel(*status);
    LOG(INFO) << __func__ << " called, response: " << response;
    std::move(callback).Run(std::move(response));
  }
}

void MemoryManagerDelegateOSE::OnLevelChanged(luna::Client::ResponseStatus,
                                              unsigned,
                                              const std::string& json) {
  if (!subscribed_)
    return;

  std::optional<base::Value> root(base::JSONReader::Read(json));
  if (!root || !root->is_dict())
    return;

  const base::Value::Dict& dict = root->GetDict();
  if (dict.FindByDottedPath("current") && dict.FindByDottedPath("previous")) {
    // Memory level is changed.
    // Example of expected json: { "previous": "normal", "current": "low" }
    const std::string* current = dict.FindStringByDottedPath("current");
    LOG(INFO) << __func__ << " calling callback with current: " << current;
    subscription_callback_.Run(std::move(*current));
  } else if (dict.FindByDottedPath("returnValue") &&
             dict.FindByDottedPath("returnValue")->GetBool()) {
    // Subscription is established.
    // Example of expected json: { "returnValue": true }
    LOG(INFO) << __func__ << " called with json: " << json;
  } else {
    LOG(ERROR) << __func__ << " wrong json value: " << json;
  }
}

}  // namespace webos
}  // namespace pal
