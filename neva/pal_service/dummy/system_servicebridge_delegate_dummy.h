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

#ifndef NEVA_PAL_SERVICE_DUMMY_SYSTEM_SERVICEBRIDGE_DELEGATE_DUMMY_H_
#define NEVA_PAL_SERVICE_DUMMY_SYSTEM_SERVICEBRIDGE_DELEGATE_DUMMY_H_

#include <string>

#include "base/functional/callback.h"
#include "neva/pal_service/public/system_servicebridge_delegate.h"

namespace pal {
namespace dummy {

class SystemServiceBridgeDelegateDummy : public SystemServiceBridgeDelegate {
 public:
  SystemServiceBridgeDelegateDummy(CreationParams params, Response callback);
  SystemServiceBridgeDelegateDummy(const SystemServiceBridgeDelegateDummy&) =
      delete;
  SystemServiceBridgeDelegateDummy& operator=(
      const SystemServiceBridgeDelegateDummy&) = delete;
  ~SystemServiceBridgeDelegateDummy() override;

  void Call(std::string uri, std::string payload) override;
  void Cancel() override;

 private:
  std::string name_;
  Response callback_;
};

}  // namespace dummy
}  // namespace pal

#endif  // NEVA_PAL_SERVICE_DUMMY_SYSTEM_SERVICEBRIDGE_DELEGATE_DUMMY_H_
