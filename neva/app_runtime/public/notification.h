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

#ifndef NEVA_APP_RUNTIME_PUBLIC_NOTIFICATION_H_
#define NEVA_APP_RUNTIME_PUBLIC_NOTIFICATION_H_

#include <memory>
#include <string>
#include <vector>

#include "neva/app_runtime/public/app_runtime_export.h"

namespace neva_app_runtime {

class NotificationEventDispatcher;

APP_RUNTIME_EXPORT NotificationEventDispatcher*
GetNotificationEventDispatcher();

class NotificationEventDispatcher {
 public:
  // Called when the desktop notification is closed. If closed by a user
  // explicitly (as opposed to timeout/script), |by_user| should be true.
  virtual void Close(const std::string& notification_id, bool by_user) = 0;

  // Called when a desktop notification is clicked. |button_index| is filled in
  // if a button was clicked (as opposed to the body of the notification) while
  // |reply| is filled in if there was an input field associated with the
  // button.
  virtual void Click(const std::string& notification_id,
                     const std::string origin,
                     const std::pair<int, bool>& button_index,
                     const std::pair<std::u16string, bool>& reply) = 0;

  virtual ~NotificationEventDispatcher() {}
};

struct ButtonInfo {
  ButtonInfo(const std::string& title, const std::string& icon_path)
      : title(title), icon_path(icon_path) {}
  std::string title;
  std::string icon_path;
};

class Notification {
 public:
  // Uniquely identifies a notification in the message center. For
  // notification front ends that support multiple profiles, this id should
  // identify a unique profile + frontend_notification_id combination. You can
  // Use this id against the MessageCenter interface but not the
  // NotificationUIManager interface.
  virtual const std::string& Id() const = 0;
  virtual const std::string& Origin() const = 0;
  virtual const std::u16string& Title() const = 0;
  virtual const std::u16string& Message() const = 0;
  virtual std::string AppId() const = 0;
  virtual const std::string& Icon() const = 0;
  virtual const std::vector<ButtonInfo>& Buttons() const = 0;

  virtual ~Notification() {}
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_PUBLIC_NOTIFICATION_H_
