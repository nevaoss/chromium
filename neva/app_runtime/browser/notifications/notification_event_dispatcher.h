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

#ifndef NEVA_APP_RUNTIME_BROWSER_NOTIFICATIONS_NOTIFICATION_EVENT_DISPATCHER_H_
#define NEVA_APP_RUNTIME_BROWSER_NOTIFICATIONS_NOTIFICATION_EVENT_DISPATCHER_H_

#include "neva/app_runtime/public/notification.h"

namespace base {
template <typename T>
struct DefaultSingletonTraits;
}

namespace neva_app_runtime {

class NotificationEventDispatcherImpl : public NotificationEventDispatcher {
 public:
  static NotificationEventDispatcherImpl* GetInstance();

  NotificationEventDispatcherImpl(const NotificationEventDispatcherImpl&) =
      delete;
  NotificationEventDispatcherImpl& operator=(
      const NotificationEventDispatcherImpl&) = delete;

  void Close(const std::string& notification_id, bool by_user) override;
  void Click(const std::string& notification_id,
             const std::string origin,
             const std::pair<int, bool>& button_index,
             const std::pair<std::u16string, bool>& reply) override;

  ~NotificationEventDispatcherImpl() override;

 private:
  NotificationEventDispatcherImpl();
  friend struct base::DefaultSingletonTraits<NotificationEventDispatcherImpl>;
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_BROWSER_NOTIFICATIONS_NOTIFICATION_EVENT_DISPATCHER_H_
