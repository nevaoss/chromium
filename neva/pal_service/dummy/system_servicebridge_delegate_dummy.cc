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

#include "neva/pal_service/dummy/system_servicebridge_delegate_dummy.h"

#include <optional>

#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/values.h"

namespace pal {
namespace dummy {

SystemServiceBridgeDelegateDummy::SystemServiceBridgeDelegateDummy(
    CreationParams params, Response callback)
    : name_(params.name.empty() ? params.appid : params.name),
      callback_(std::move(callback)) {
}

SystemServiceBridgeDelegateDummy::~SystemServiceBridgeDelegateDummy() {}

void SystemServiceBridgeDelegateDummy::Call(std::string uri,
                                            std::string payload) {
  base::Value::Dict dict;
  dict.Set("name", name_);
  dict.Set("method", __FUNCTION__);
  dict.Set("uri", uri);

  std::optional<base::Value> payload_value = base::JSONReader::Read(payload);
  if (payload_value && payload_value->is_dict())
    dict.Set("payload", std::move(*payload_value));

  std::string response;
  if (base::JSONWriter::Write(dict, &response))
    callback_.Run(pal::mojom::ResponseStatus::kSuccess, std::move(response));
  else
    NOTREACHED();
}

void SystemServiceBridgeDelegateDummy::Cancel() {
  base::Value::Dict dict;
  dict.Set("name", name_);
  dict.Set("method", __FUNCTION__);

  std::string response;
  if (base::JSONWriter::Write(dict, &response))
    callback_.Run(pal::mojom::ResponseStatus::kCanceled, std::move(response));
  else
    NOTREACHED();
}

}  // namespace dummy
}  // namespace pal
