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

#ifndef UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_TEST_TEST_WEBOS_EXTENSION_SERVER_THREAD_H_
#define UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_TEST_TEST_WEBOS_EXTENSION_SERVER_THREAD_H_

#include <memory>
#include <vector>

#include <wayland-server-core.h>

#include "base/message_loop/message_pump_epoll.h"
#include "base/synchronization/waitable_event.h"
#include "base/threading/thread.h"
#include "ui/ozone/platform/wayland/extensions/webos/test/mock_shell.h"
#include "ui/ozone/platform/wayland/extensions/webos/test/test_webos_extension_compositor.h"
#include "ui/ozone/platform/wayland/test/global_object.h"
#include "ui/ozone/platform/wayland/test/test_output.h"
#include "ui/ozone/platform/wayland/test/test_seat.h"

struct wl_client;
struct wl_display;
struct wl_event_loop;
struct wl_resource;

namespace wl {

class TestWebosExtensionServerThread : public TestOutput::Delegate,
                                       public base::Thread,
                                       base::MessagePumpEpoll::FdWatcher {
 public:
  TestWebosExtensionServerThread();
  ~TestWebosExtensionServerThread() override;

  // Starts the test Wayland server thread. If this succeeds, the WAYLAND_SOCKET
  // environment variable will be set to the string representation of a file
  // descriptor that a client can connect to. The caller is responsible for
  // ensuring that this file descriptor gets closed (for example, by calling
  // wl_display_connect).
  // Instantiates webOS extension interfaces.
  bool Start();

  // Pauses the server thread when it becomes idle.
  void Pause();

  // Resumes the server thread after flushing client connections.
  void Resume();

  template <typename T>
  T* GetObject(uint32_t id) {
    wl_resource* resource = wl_client_get_object(client_, id);
    return resource ? T::FromResource(resource) : nullptr;
  }

  TestOutput* CreateAndInitializeOutput(TestOutputMetrics metrics = {}) {
    auto output = std::make_unique<TestOutput>(this, std::move(metrics));
    output->Initialize(display());

    TestOutput* output_ptr = output.get();
    globals_.push_back(std::move(output));
    return output_ptr;
  }

  // TestOutput::Delegate:
  void OnTestOutputFlush(TestOutput* test_output,
                         const TestOutputMetrics& metrics) override;
  void OnTestOutputGlobalDestroy(TestOutput* test_output) override;

  // Called when the Flush() is called for a TestOutput associated with
  // `output_resource`.
  void OnTestOutputMetricsFlush(wl_resource* output_resource,
                                const TestOutputMetrics& metrics);

  TestSeat* seat() { return &seat_; }
  MockShell* shell() { return &shell_; }
  MockWebosShell* webos_shell() { return &webos_shell_; }
  TestOutput* output() { return &output_; }
  wl_display* display() const { return display_.get(); }

  TestWebosExtensionServerThread(const TestWebosExtensionServerThread&) =
      delete;
  TestWebosExtensionServerThread& operator=(
      const TestWebosExtensionServerThread&) = delete;

 private:
  struct DisplayDeleter {
    void operator()(wl_display* display);
  };

  void DoPause();

  std::unique_ptr<base::MessagePump> CreateMessagePump();

  // base::MessagePumpEpoll::FdWatcher
  void OnFileCanReadWithoutBlocking(int fd) override;
  void OnFileCanWriteWithoutBlocking(int fd) override;

  std::unique_ptr<wl_display, DisplayDeleter> display_;
  wl_client* client_ = nullptr;
  wl_event_loop* event_loop_ = nullptr;

  base::WaitableEvent pause_event_;
  base::WaitableEvent resume_event_;

  // Wayland global objects
  TestWebosExtensionCompositor compositor_;
  TestOutput output_;
  TestSeat seat_;
  MockShell shell_;
  MockWebosShell webos_shell_;

  std::vector<std::unique_ptr<GlobalObject>> globals_;

  base::MessagePumpEpoll::FdWatchController controller_;
};

}  // namespace wl

#endif  // UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_TEST_TEST_WEBOS_EXTENSION_SERVER_THREAD_H_
