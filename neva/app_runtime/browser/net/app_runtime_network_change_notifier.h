// Copyright 2018 LG Electronics, Inc.
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

#ifndef NEVA_APP_RUNTIME_BROWSER_NET_APP_RUNTIME_NETWORK_CHANGE_NOTIFIER_H_
#define NEVA_APP_RUNTIME_BROWSER_NET_APP_RUNTIME_NETWORK_CHANGE_NOTIFIER_H_

#include "net/base/network_change_notifier.h"

namespace neva_app_runtime {

class AppRuntimeNetworkChangeNotifier : public net::NetworkChangeNotifier {
 public:
  AppRuntimeNetworkChangeNotifier();
  AppRuntimeNetworkChangeNotifier(const AppRuntimeNetworkChangeNotifier&) =
      delete;
  AppRuntimeNetworkChangeNotifier& operator=(
      const AppRuntimeNetworkChangeNotifier&) = delete;
  void OnNetworkStateChanged(bool is_connected);

  // net::NetworkChangeNotifier overrides.
  net::NetworkChangeNotifier::ConnectionType GetCurrentConnectionType()
      const override;

  static AppRuntimeNetworkChangeNotifier* GetInstance();

 private:
  bool network_connected_ = true;
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_BROWSER_NET_APP_RUNTIME_NETWORK_CHANGE_NOTIFIER_H_
