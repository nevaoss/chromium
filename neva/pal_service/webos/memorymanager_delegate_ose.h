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

#ifndef NEVA_PAL_SERVICE_WEBOS_MEMORYMANAGER_DELEGATE_OSE_H_
#define NEVA_PAL_SERVICE_WEBOS_MEMORYMANAGER_DELEGATE_OSE_H_

#include "neva/pal_service/webos/memorymanager_delegate_webos.h"

namespace pal {
namespace webos {

class MemoryManagerDelegateOSE : public MemoryManagerDelegateWebOS {
 public:
  MemoryManagerDelegateOSE();
  MemoryManagerDelegateOSE(const MemoryManagerDelegateOSE&) = delete;
  MemoryManagerDelegateOSE& operator=(const MemoryManagerDelegateOSE&) = delete;
  ~MemoryManagerDelegateOSE() override;

  void GetMemoryStatus(OnceResponse callback) override;
  void SubscribeToLevelChanged(RepeatingResponse callback) override;

 private:
  void OnMemoryStatus(OnceResponse callback,
                      luna::Client::ResponseStatus,
                      unsigned token,
                      const std::string& json);
  void OnLevelChanged(luna::Client::ResponseStatus,
                      unsigned token,
                      const std::string& json);

  base::WeakPtr<MemoryManagerDelegateOSE> weak_ptr_this_;
  base::WeakPtrFactory<MemoryManagerDelegateOSE> weak_factory_;
};

}  // namespace webos
}  // namespace pal

#endif  // NEVA_PAL_SERVICE_WEBOS_MEMORYMANAGER_DELEGATE_OSE_H_
