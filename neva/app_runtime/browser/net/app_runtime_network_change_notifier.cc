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

#include "neva/app_runtime/browser/net/app_runtime_network_change_notifier.h"

namespace neva_app_runtime {

namespace {

AppRuntimeNetworkChangeNotifier* g_network_chnage_notifier = nullptr;

}

// wam_demo is the only one using AppRuntimeNetworkChangeNotifier::GetInstance
// to distribute notifications received externally.
//
// The lifetime of the AppRuntimeNetworkChangeNotifier instance is managed by
// BrowserMainLoop. The instance of AppRuntimeNetworkChangeNotifier is created
// once with AppRuntimeNetworkChangeNotifierFactory::CreateInstance, which
// returns unique_ptr<AppRuntimeNetworkChangeNotifierFactory>.
//
// AppRuntimeNetworkChangeNotifier repeats the global single instance pointer
// scheme made by net::NetworkChangeNotifier, where "this" is assigned to it in
// the NetworkChangeNotifier constructor.
//
// Thus we have two global pointers neva_app_runtime::g_network_chnage_notifier
// referring to an instance of AppRuntimeNetworkChangeNotifier and
// net::g_network_change_notifier referring to an net::NetworkChangeNotifier
// instance, with AppRuntimeNetworkChangeNotifier inheriting
// NetworkChangeNotifier, and neva_app_runtime::g_network_change_notifier and
// net::g_network_change_notifier referring through different types to the same
// instance.

AppRuntimeNetworkChangeNotifier::AppRuntimeNetworkChangeNotifier() {
  g_network_chnage_notifier = this;
}

// static
AppRuntimeNetworkChangeNotifier*
AppRuntimeNetworkChangeNotifier::GetInstance() {
  return g_network_chnage_notifier;
}

net::NetworkChangeNotifier::ConnectionType
AppRuntimeNetworkChangeNotifier::GetCurrentConnectionType() const {
  return network_connected_ ? net::NetworkChangeNotifier::CONNECTION_UNKNOWN
                            : net::NetworkChangeNotifier::CONNECTION_NONE;
}

void AppRuntimeNetworkChangeNotifier::OnNetworkStateChanged(
    bool is_connected) {
  if (network_connected_ != is_connected) {
    network_connected_ = is_connected;
    net::NetworkChangeNotifier::NotifyObserversOfMaxBandwidthChange(
        network_connected_ ? std::numeric_limits<double>::infinity() : 0.0,
        GetCurrentConnectionType());
  }
}

}  // namespace neva_app_runtime
