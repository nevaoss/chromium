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

// Based on
// //chrome/browser/notifications/displayed_notifications_dispatch_callback.h

// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file

#ifndef NEVA_APP_RUNTIME_PUBLIC_DISPLAYED_NOTIFICATIONS_DISPATCH_CALLBACK_H_
#define NEVA_APP_RUNTIME_PUBLIC_DISPLAYED_NOTIFICATIONS_DISPATCH_CALLBACK_H_

#include <set>
#include <string>

#include "neva/app_runtime/public/callback_helper.h"

namespace neva_app_runtime {

// Callback used by the bridge and all the downstream classes that propagate
// the callback to get displayed notifications.
//
// |supports_synchronization| will be true if the platform supports getting the
// currently displayed notifications.
//
// If |supports_synchronization| is true, then |notification_ids| will contain
// the ids of the currently displayed notifications, otherwise the value of
// |notification_ids| should be ignored.
using GetDisplayedNotificationsCallback =
    CallbackHelper<void(std::set<std::string> notification_ids,
                        bool supports_synchronization)>;

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_PUBLIC_DISPLAYED_NOTIFICATIONS_DISPATCH_CALLBACK_H_
