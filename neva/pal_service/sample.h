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

#ifndef NEVA_PAL_SERVICE_SAMPLE_H_
#define NEVA_PAL_SERVICE_SAMPLE_H_

#include "base/functional/callback.h"
#include "mojo/public/cpp/bindings/receiver_set.h"
#include "mojo/public/cpp/bindings/remote_set.h"
#include "neva/emulator/emulator_data_source.h"
#include "neva/pal_service/public/mojom/sample.mojom.h"

namespace pal {

class SampleImpl : public mojom::Sample
                 , public emulator::EmulatorDataDelegate {
 public:
  SampleImpl();
  SampleImpl(const SampleImpl&) = delete;
  SampleImpl& operator=(const SampleImpl&) = delete;
  ~SampleImpl() override;

  void AddBinding(mojo::PendingReceiver<mojom::Sample> receiver);

  // mojom::Sample
  void CurrentValue(CurrentValueCallback callback) override;
  void PlatformValue(PlatformValueCallback callback) override;
  void CallFunc(const std::string& arg1, const std::string& arg2) override;
  void ProcessData(const std::string& data,
                   ProcessDataCallback callback) override;
  void Subscribe(SubscribeCallback callback) override;

 private:
  // from EmulatorDataDelegate
  void DataUpdated(const std::string& url, const std::string& data) override;
  void onProcessDataResponse(const std::string& data);

  mojo::ReceiverSet<mojom::Sample> receivers_;

  int process_data_req_id_ = 0;
  using ProcessDataRequests = std::map<int, ProcessDataCallback>;
  ProcessDataRequests process_data_requests_;

  mojo::AssociatedRemoteSet<mojom::SampleListener> remote_listeners_;
};

}  // namespace pal

#endif  // #define NEVA_PAL_SERVICE_SAMPLE_H_
