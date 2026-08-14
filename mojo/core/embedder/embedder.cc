// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/core/embedder/embedder.h"

#include <stdint.h>

#include <utility>

#include "base/check.h"
#include "base/feature_list.h"
#include "base/memory/ref_counted.h"
#include "base/notreached.h"
#include "base/task/single_thread_task_runner.h"
#include "base/task/task_runner.h"
#include "build/build_config.h"
#include "mojo/buildflags.h"
#include "mojo/core/channel.h"
#include "mojo/core/configuration.h"
#include "mojo/core/core_ipcz.h"
#include "mojo/core/embedder/features.h"
#include "mojo/core/ipcz_api.h"
#include "mojo/core/ipcz_driver/base_shared_memory_service.h"
#include "mojo/core/ipcz_driver/driver.h"
#include "mojo/core/ipcz_driver/transport.h"
#include "mojo/public/c/system/thunks.h"

#if BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS) || BUILDFLAG(IS_ANDROID)
#include "mojo/core/channel_linux.h"
#endif  // BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS) ||
        // BUILDFLAG(IS_ANDROID)

namespace mojo::core {

namespace {

<<<<<<< HEAD
#if BUILDFLAG(MOJO_SUPPORT_LEGACY_CORE)
// TODO(neva): Remove BUILDFLAG(IS_WEBOS) if mojoipcz is better when considering
// memory and performance perspective.
#if BUILDFLAG(IS_CHROMEOS) && !defined(ENABLE_IPCZ_ON_CHROMEOS) || \
    BUILDFLAG(IS_WEBOS)
std::atomic<bool> g_mojo_ipcz_enabled{false};
#else
// Default to enabled even if InitFeatures() is never called.
std::atomic<bool> g_mojo_ipcz_enabled{true};
#endif

bool g_mojo_ipcz_force_disabled = false;

std::optional<std::string> GetMojoIpczEnvVar() {
  auto env = base::Environment::Create();
  return env->GetVar("MOJO_IPCZ");
}

// Allows MojoIpcz to be forcibly enabled if and only if MOJO_IPCZ=1 in the
// environment. Note that any other value (or absence) has no influence on
// whether or not MojoIpcz is enabled.
bool IsMojoIpczForceEnabledByEnvironment() {
  static bool force_enabled = GetMojoIpczEnvVar() == "1";
  return force_enabled;
}
#endif  // BUILDFLAG(MOJO_SUPPORT_LEGACY_CORE)

=======
>>>>>>> 153.0.7981.0~1
bool g_enable_memv2 = false;

}  // namespace

// InitFeatures will be called as soon as the base::FeatureList is initialized.
void InitFeatures() {
  CHECK(base::FeatureList::GetInstance());

#if BUILDFLAG(IS_POSIX) && !BUILDFLAG(MOJO_USE_APPLE_CHANNEL)
#if BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS) || BUILDFLAG(IS_ANDROID)
  bool shared_mem_enabled = base::FeatureList::IsEnabled(kMojoUseEventFd);
  int num_pages = kMojoUseEventFdPages.Get();
  if (num_pages < 0) {
    num_pages = 4;
  } else if (num_pages > 128) {
    num_pages = 128;
  }

  ChannelLinux::SetSharedMemParameters(shared_mem_enabled,
                                       static_cast<unsigned int>(num_pages));
#endif  // BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS) ||
        // BUILDFLAG(IS_ANDROID)
#endif  // BUILDFLAG(IS_POSIX) && !BUILDFLAG(IS_MAC)

  g_enable_memv2 = base::FeatureList::IsEnabled(kMojoIpczMemV2);
}

void Init(const Configuration& configuration) {
  internal::g_configuration = configuration;

  CHECK(!configuration.disable_ipcz);

  if (IsMojoIpczEnabled()) {
    CHECK(InitializeIpczNodeForProcess({
        .is_broker = configuration.is_broker_process,
        .use_local_shared_memory_allocation =
            configuration.is_broker_process ||
            configuration.force_direct_shared_memory_allocation,
        .enable_memv2 = g_enable_memv2,
    }));
    MojoEmbedderSetSystemThunks(GetMojoIpczImpl());
  } else {
    NOTREACHED();
  }
}

void Init() {
  Init(Configuration());
}

void ShutDown() {
  if (IsMojoIpczEnabled()) {
    DestroyIpczNodeForProcess();
  } else {
    NOTREACHED();
  }
}

scoped_refptr<base::SingleThreadTaskRunner> GetIOTaskRunner() {
  if (IsMojoIpczEnabled()) {
    return ipcz_driver::Transport::GetIOTaskRunner();
  } else {
    NOTREACHED();
  }
}

bool IsMojoIpczEnabled() {
  return true;
}

void InstallMojoIpczBaseSharedMemoryHooks() {
  DCHECK(IsMojoIpczEnabled());
  ipcz_driver::BaseSharedMemoryService::InstallHooks();
}

const IpczAPI& GetIpczAPIForMojo() {
  return GetIpczAPI();
}

const IpczDriver& GetIpczDriverForMojo() {
  return ipcz_driver::GetIpczDriver();
}

IpczDriverHandle CreateIpczTransportFromEndpoint(
    mojo::PlatformChannelEndpoint endpoint,
    const TransportEndpointTypes& endpoint_types,
    base::Process remote_process) {
  auto transport = ipcz_driver::Transport::Create(
      {
          .source = endpoint_types.local_is_broker
                        ? ipcz_driver::Transport::kBroker
                        : ipcz_driver::Transport::kNonBroker,
          .destination = endpoint_types.remote_is_broker
                             ? ipcz_driver::Transport::kBroker
                             : ipcz_driver::Transport::kNonBroker,
      },
      std::move(endpoint), std::move(remote_process));
  return ipcz_driver::ObjectBase::ReleaseAsHandle(std::move(transport));
}

}  // namespace mojo::core
