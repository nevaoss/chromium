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

#ifndef NEVA_PAL_SERVICE_PUBLIC_NOTIFICATION_MANAGER_DELEGATE_H_
#define NEVA_PAL_SERVICE_PUBLIC_NOTIFICATION_MANAGER_DELEGATE_H_

#include <string>

namespace pal {

class NotificationManagerDelegate {
 public:
  virtual ~NotificationManagerDelegate() = default;
  virtual void CreateToast(const std::string& app_id,
                           const std::string& msg) = 0;
};

}  // namespace pal

#endif  // NEVA_PAL_SERVICE_PUBLIC_NOTIFICATION_MANAGER_DELEGATE_H_
