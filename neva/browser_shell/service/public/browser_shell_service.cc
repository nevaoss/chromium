// Copyright 2021 LG Electronics, Inc.
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

#include "neva/browser_shell/service/public/browser_shell_service.h"

#include <memory>

#include "base/functional/bind.h"
#include "base/logging.h"
#include "base/no_destructor.h"
#include "base/task/task_traits.h"
#include "base/task/thread_pool.h"
#include "base/threading/sequence_local_storage_slot.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "mojo/public/cpp/bindings/receiver_set.h"
#include "neva/logging.h"

namespace browser_shell {

namespace {

std::unique_ptr<mojom::ShellService>& GetServiceFromSequenceLocalStorage() {
  static base::SequenceLocalStorageSlot<std::unique_ptr<mojom::ShellService>>
      service_slot;
  return service_slot.GetOrCreateValue();
}

void DoRegisterShellService(
    std::unique_ptr<mojom::ShellService> shell_service) {
  auto& service = GetServiceFromSequenceLocalStorage();
  NEVA_DCHECK(!service);
  if (!service)
    service = std::move(shell_service);
}

}  // anonymous namespace

void RegisterShellService(std::unique_ptr<mojom::ShellService> shell_service) {
  auto& service = GetServiceFromSequenceLocalStorage();
  NEVA_DCHECK(!service);
  if (!service) {
    LOG(INFO) << "Register Browser ShellService";
    content::GetUIThreadTaskRunner({})->PostTask(
        FROM_HERE,
        base::BindOnce(&DoRegisterShellService, std::move(shell_service)));
  }
}

void BindShellServiceReceiver(
    mojo::PendingReceiver<mojom::ShellService> receiver) {
  auto& service = GetServiceFromSequenceLocalStorage();
  NEVA_DCHECK(service);
  if (service)
    service->AddBinding(std::move(receiver));
  else
    LOG(ERROR) << "Cannot bind Browser ShellService";
}

}  // namespace browser_shell
