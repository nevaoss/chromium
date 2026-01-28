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

#include "neva/pal_service/sample.h"

#include "content/public/browser/browser_thread.h"
#include "neva/emulator/emulator_urls.h"

namespace pal {

using namespace emulator;

SampleImpl::SampleImpl() {
  EmulatorDataSource* pEmulatorInterface = EmulatorDataSource::GetInstance();

  // adding all URLs (to different entities of the Sample injection verified)
  const char* poll_urls[] = {kSample_getPlatformValue, kSample_sampleUpdate,
                             kSample_processDataResponse};

  for (auto* url : poll_urls) {
    pEmulatorInterface->AddURLForPolling(
        url, this, base::SingleThreadTaskRunner::GetCurrentDefault());
  }
}

SampleImpl::~SampleImpl() {}

void SampleImpl::AddBinding(mojo::PendingReceiver<mojom::Sample> receiver) {
  receivers_.Add(this, std::move(receiver));
}

void SampleImpl::CurrentValue(CurrentValueCallback callback) {
  std::move(callback).Run(std::string("0"));
}

void SampleImpl::PlatformValue(PlatformValueCallback callback) {
  std::string platform_value =
      EmulatorDataSource::GetInstance()->GetCachedValueForURL(
          kSample_getPlatformValue);
  std::move(callback).Run(std::move(platform_value));
}

void SampleImpl::CallFunc(const std::string& arg1, const std::string& arg2) {
  RequestArgs args_vector = {
    { "arg1", &arg1 },
    { "arg2", &arg2 }
  };

  std::string params = EmulatorDataSource::PrepareRequestParams(args_vector);
  EmulatorDataSource::SetExpectationAsync(kSample_callFunc, std::move(params));
}

void SampleImpl::ProcessData(const std::string& data,
                             ProcessDataCallback callback) {
  std::string id(std::to_string(++process_data_req_id_));
  process_data_requests_.insert(
      std::make_pair(process_data_req_id_, std::move(callback)));

  RequestArgs args_vector = {
    { "id", &id },
    { "data", &data }
  };

  std::string params = EmulatorDataSource::PrepareRequestParams(args_vector);
  EmulatorDataSource::SetExpectationAsync(kSample_processDataReq,
                                          std::move(params));
}

void SampleImpl::Subscribe(SubscribeCallback callback) {
  mojo::AssociatedRemote<mojom::SampleListener> listener;
  std::move(callback).Run(listener.BindNewEndpointAndPassReceiver());
  remote_listeners_.Add(std::move(listener));
}

void SampleImpl::onProcessDataResponse(const std::string& data) {
  std::string id;
  std::string result;
  ResponseArgs args_vector = {
    { "id", &id },
    { "result", &result }
  };

  if (!EmulatorDataSource::GetResponseParams(args_vector, data))
    return;

  auto it = process_data_requests_.find(std::stoi(id));
  if (it == process_data_requests_.end()) {
    LOG(ERROR) << __func__ << "(): incorrect response id = " << id;
    return;
  }

  const bool ret = result.compare("false") != 0;
  std::move(it->second).Run(ret);
  process_data_requests_.erase(it);
}

void SampleImpl::DataUpdated(const std::string& url,
                             const std::string& data) {
  if (url == kSample_processDataResponse) {
    onProcessDataResponse(data);
  } else if (url == kSample_sampleUpdate) {
    for (auto& listener : remote_listeners_)
      listener->DataUpdated(data);
  }
}

}  // namespace pal
