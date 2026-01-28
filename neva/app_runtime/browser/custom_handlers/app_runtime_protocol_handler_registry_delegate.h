// Copyright 2023 LG Electronics, Inc.
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
// //components/custom_handlers/test_protocol_handler_registry_delegate.h

// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NEVA_APP_RUNTIME_BROWSER_CUSTOM_HANDLERS_APP_RUNTIME_PROTOCOL_HANDLER_REGISTRY_DELEGATE_H_
#define NEVA_APP_RUNTIME_BROWSER_CUSTOM_HANDLERS_APP_RUNTIME_PROTOCOL_HANDLER_REGISTRY_DELEGATE_H_

#include <set>
#include <string>

#include "components/custom_handlers/protocol_handler_registry.h"

namespace neva_app_runtime {

// A test ProtocolHandlerRegistry::Delegate implementation that keeps track of
// registered protocols and doesn't change any OS settings.
class AppRuntimeProtocolHandlerRegistryDelegate
    : public custom_handlers::ProtocolHandlerRegistry::Delegate {
 public:
  AppRuntimeProtocolHandlerRegistryDelegate();
  ~AppRuntimeProtocolHandlerRegistryDelegate() override;

  AppRuntimeProtocolHandlerRegistryDelegate(
      const AppRuntimeProtocolHandlerRegistryDelegate& other) = delete;
  AppRuntimeProtocolHandlerRegistryDelegate& operator=(
      const AppRuntimeProtocolHandlerRegistryDelegate& other) = delete;

  // ProtocolHandlerRegistry::Delegate:
  void RegisterExternalHandler(const std::string& protocol) override;
  void DeregisterExternalHandler(const std::string& protocol) override;
  bool IsExternalHandlerRegistered(const std::string& protocol) override;
  void RegisterWithOSAsDefaultClient(const std::string& protocol,
                                     DefaultClientCallback callback) override;
  void CheckDefaultClientWithOS(const std::string& protocol,
                                DefaultClientCallback callback) override;
  bool ShouldRemoveHandlersNotInOS() override;

 private:
  // Holds registered protocols.
  std::set<std::string> registered_protocols_;
  std::set<std::string> os_registered_protocols_;
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_BROWSER_CUSTOM_HANDLERS_APP_RUNTIME_PROTOCOL_HANDLER_REGISTRY_DELEGATE_H_
