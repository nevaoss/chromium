// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_OZONE_PUBLIC_GPU_PLATFORM_SUPPORT_H_
#define UI_OZONE_PUBLIC_GPU_PLATFORM_SUPPORT_H_

#include "base/component_export.h"
#include "ipc/ipc_listener.h"
#include "ipc/ipc_sender.h"

namespace IPC {
class MessageFilter;
}

namespace ui {

// Platform-specific object to support a GPU process.
//
// See GpuPlatformSupportHost for more context.
class COMPONENT_EXPORT(OZONE_BASE) GpuPlatformSupport : public IPC::Listener {
 public:
  GpuPlatformSupport();
  ~GpuPlatformSupport() override;

  // Called when the GPU process is spun up & channel established.
  virtual void OnChannelEstablished(IPC::Sender* sender) = 0;

  virtual IPC::MessageFilter* GetMessageFilter() = 0;
};

// Create a stub implementation.
COMPONENT_EXPORT(OZONE_BASE) GpuPlatformSupport* CreateStubGpuPlatformSupport();

}  // namespace ui

#endif  // UI_OZONE_PUBLIC_GPU_PLATFORM_SUPPORT_H_
