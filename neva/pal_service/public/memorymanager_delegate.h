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

#ifndef NEVA_PAL_SERVICE_PUBLIC_MEMORYMANAGER_DELEGATE_H_
#define NEVA_PAL_SERVICE_PUBLIC_MEMORYMANAGER_DELEGATE_H_

#include <string>
#include "base/functional/callback.h"

namespace pal {
extern const char kMemoryLevelCritical[];
extern const char kMemoryLevelLow[];
extern const char kMemoryLevelNormal[];

class MemoryManagerDelegate {
 public:
  virtual ~MemoryManagerDelegate() {}

  using OnceResponse = base::OnceCallback<void (const std::string&)>;
  virtual void GetMemoryStatus(OnceResponse callback) = 0;

  using RepeatingResponse = base::RepeatingCallback<void (const std::string&)>;
  virtual void SubscribeToLevelChanged(RepeatingResponse callback) = 0;

  virtual void UnsubscribeFromLevelChanged() = 0;

  virtual bool IsSubscribed() const = 0;
};

}  // namespace pal

#endif  // NEVA_PAL_SERVICE_PUBLIC_MEMORYMANAGER_DELEGATE_H_
