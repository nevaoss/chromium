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

#include "neva/app_runtime/browser/notifications/notification_event_dispatcher.h"

#include "base/memory/singleton.h"
#include "base/notreached.h"

namespace neva_app_runtime {

NotificationEventDispatcher* GetNotificationEventDispatcher() {
  return NotificationEventDispatcherImpl::GetInstance();
}

// static
NotificationEventDispatcherImpl*
NotificationEventDispatcherImpl::GetInstance() {
  return base::Singleton<NotificationEventDispatcherImpl>::get();
}

NotificationEventDispatcherImpl::NotificationEventDispatcherImpl() = default;

NotificationEventDispatcherImpl::~NotificationEventDispatcherImpl() = default;

void NotificationEventDispatcherImpl::Close(const std::string& notification_id,
                                            bool by_user) {
  NOTIMPLEMENTED();
}

void NotificationEventDispatcherImpl::Click(
    const std::string& notification_id,
    const std::string origin,
    const std::pair<int, bool>& button_index,
    const std::pair<std::u16string, bool>& reply) {
  NOTIMPLEMENTED();
}

}  // namespace neva_app_runtime
