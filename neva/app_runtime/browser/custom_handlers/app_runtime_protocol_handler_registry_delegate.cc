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
// //components/custom_handlers/test_protocol_handler_registry_delegate.cc

// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "neva/app_runtime/browser/custom_handlers/app_runtime_protocol_handler_registry_delegate.h"

#include <utility>

#include "base/functional/bind.h"
#include "base/task/single_thread_task_runner.h"
#include "neva/logging.h"

namespace neva_app_runtime {

AppRuntimeProtocolHandlerRegistryDelegate::
    AppRuntimeProtocolHandlerRegistryDelegate() = default;

AppRuntimeProtocolHandlerRegistryDelegate::
    ~AppRuntimeProtocolHandlerRegistryDelegate() = default;

// ProtocolHandlerRegistry::Delegate:
void AppRuntimeProtocolHandlerRegistryDelegate::RegisterExternalHandler(
    const std::string& protocol) {
  bool inserted = registered_protocols_.insert(protocol).second;
  NEVA_DCHECK(inserted);
}

void AppRuntimeProtocolHandlerRegistryDelegate::DeregisterExternalHandler(
    const std::string& protocol) {
  size_t removed = registered_protocols_.erase(protocol);
  NEVA_DCHECK(removed == 1u);
}

bool AppRuntimeProtocolHandlerRegistryDelegate::IsExternalHandlerRegistered(
    const std::string& protocol) {
  return registered_protocols_.find(protocol) != registered_protocols_.end();
}

void AppRuntimeProtocolHandlerRegistryDelegate::RegisterWithOSAsDefaultClient(
    const std::string& protocol,
    DefaultClientCallback callback) {
  // Do as-if the registration has to run on another sequence and post back
  // the result with a task to the current thread.
  base::SingleThreadTaskRunner::GetCurrentDefault()->PostTask(
      FROM_HERE, base::BindOnce(std::move(callback), true));

  os_registered_protocols_.insert(protocol);
}

void AppRuntimeProtocolHandlerRegistryDelegate::CheckDefaultClientWithOS(
    const std::string& protocol,
    DefaultClientCallback callback) {
  // Respond asynchronously to mimic the real behavior.
  base::SingleThreadTaskRunner::GetCurrentDefault()->PostTask(
      FROM_HERE, base::BindOnce(std::move(callback), true));
}

bool AppRuntimeProtocolHandlerRegistryDelegate::ShouldRemoveHandlersNotInOS() {
  return false;
}

}  // namespace neva_app_runtime
