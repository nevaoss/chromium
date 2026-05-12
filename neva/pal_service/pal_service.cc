// Copyright 2019 LG Electronics, Inc.
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

#include "neva/pal_service/pal_service.h"

#include <map>
#include <memory>
#include <string>

#include "base/functional/bind.h"
#include "base/no_destructor.h"
#include "base/task/task_traits.h"
#include "base/task/thread_pool.h"
#include "base/threading/sequence_local_storage_slot.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "neva/pal_service/memorymanager.h"
#include "neva/pal_service/network_error_page_controller.h"
#include "neva/pal_service/public/mojom/memorymanager.mojom.h"
#include "neva/pal_service/public/mojom/network_error_page_controller.mojom.h"
#include "neva/pal_service/public/mojom/sample.mojom.h"
#include "neva/pal_service/public/mojom/system_servicebridge.mojom.h"
#include "neva/pal_service/sample.h"
#include "neva/pal_service/system_servicebridge.h"

namespace pal {

class PalServiceImpl : public mojom::PalService {
 public:
  PalServiceImpl(mojo::PendingReceiver<mojom::PalService> receiver);
  ~PalServiceImpl() override;

 private:
  void BindMemoryManager(
      mojo::PendingReceiver<mojom::MemoryManager> receiver) override;
  void BindNetworkErrorPageController(
      mojo::PendingReceiver<mojom::NetworkErrorPageController> receiver)
      override;
  void BindSample(mojo::PendingReceiver<mojom::Sample> receiver) override;
  void BindSystemServiceBridgeProvider(
      mojo::PendingReceiver<mojom::SystemServiceBridgeProvider>
          receiver) override;

  std::unique_ptr<pal::MemoryManagerImpl> memorymanager_impl_;
  std::unique_ptr<pal::NetworkErrorPageControllerImpl>
      network_error_page_controller_impl_;
  std::unique_ptr<pal::SampleImpl> sample_impl_;
  std::unique_ptr<pal::SystemServiceBridgeProviderImpl>
      system_servicebridge_provider_impl_;

  mojo::Receiver<mojom::PalService> receiver_;
};

PalServiceImpl::PalServiceImpl(
    mojo::PendingReceiver<mojom::PalService> receiver)
    : receiver_(this, std::move(receiver)) {
}

PalServiceImpl::~PalServiceImpl() {
}

void PalServiceImpl::BindMemoryManager(
    mojo::PendingReceiver<mojom::MemoryManager> receiver) {
  if (!memorymanager_impl_)
    memorymanager_impl_ = std::make_unique<MemoryManagerImpl>();
  memorymanager_impl_->AddBinding(std::move(receiver));
}

void PalServiceImpl::BindNetworkErrorPageController(
    mojo::PendingReceiver<mojom::NetworkErrorPageController> receiver) {
  if (!network_error_page_controller_impl_) {
    network_error_page_controller_impl_ =
        std::make_unique<NetworkErrorPageControllerImpl>();
  }

  network_error_page_controller_impl_->AddBinding(std::move(receiver));
}

void PalServiceImpl::BindSample(mojo::PendingReceiver<mojom::Sample> receiver) {
  if (!sample_impl_)
    sample_impl_ = std::make_unique<SampleImpl>();
  sample_impl_->AddBinding(std::move(receiver));
}

void PalServiceImpl::BindSystemServiceBridgeProvider(
    mojo::PendingReceiver<mojom::SystemServiceBridgeProvider> receiver) {
  if (!system_servicebridge_provider_impl_) {
    system_servicebridge_provider_impl_ =
        std::make_unique<SystemServiceBridgeProviderImpl>();
  }
  system_servicebridge_provider_impl_->AddBinding(std::move(receiver));
}

namespace {

std::unique_ptr<PalServiceImpl> CreatePalService(
    mojo::PendingReceiver<mojom::PalService> receiver) {
  return std::make_unique<PalServiceImpl>(std::move(receiver));
}

void BindPalServiceReceiver(
    mojo::PendingReceiver<mojom::PalService> receiver) {
  // Bind the lifetime of the service instance to that of the sequence it's
  // running on.
  static base::SequenceLocalStorageSlot<std::unique_ptr<pal::PalServiceImpl>>
      service_slot;
  auto& service = service_slot.GetOrCreateValue();

  // This function should only be called once during the lifetime of the
  // service's bound sequence.
  DCHECK(!service);

  service = CreatePalService(std::move(receiver));
}

}  // anonymous namespace

pal::mojom::PalService& GetPalService(
    const scoped_refptr<base::SingleThreadTaskRunner>& task_manager) {
  using namespace content;
  static base::SequenceLocalStorageSlot<mojo::Remote<mojom::PalService>>
      remote_slot;
  mojo::Remote<mojom::PalService>& remote =
      remote_slot.GetOrCreateValue();
  if (!remote) {
    // To be sure that PalService gets screated after main message loop
    // has been started
    task_manager->PostTask(FROM_HERE,
                           base::BindOnce(&BindPalServiceReceiver,
                                          remote.BindNewPipeAndPassReceiver()));
  }
  return *remote.get();
}

}  // namespace pal
