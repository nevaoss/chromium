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

#include "ui/ozone/platform/wayland/extensions/webos/test/test_webos_extension_server_thread.h"

#include <stdlib.h>
#include <memory>

#include <sys/socket.h>
#include <wayland-server.h>

#include "base/files/file_util.h"
#include "base/files/scoped_file.h"
#include "base/functional/bind.h"
#include "base/run_loop.h"
#include "base/strings/string_number_conversions.h"

namespace wl {

namespace {

constexpr const char* kMockThreadName = "test_webos_extension_server";
constexpr const char* kWaylandEnvVarName = "WAYLAND_SOCKET";

}  // namespace

void TestWebosExtensionServerThread::DisplayDeleter::operator()(
    wl_display* display) {
  wl_display_destroy(display);
}

TestWebosExtensionServerThread::TestWebosExtensionServerThread()
    : Thread(kMockThreadName),
      pause_event_(base::WaitableEvent::ResetPolicy::AUTOMATIC,
                   base::WaitableEvent::InitialState::NOT_SIGNALED),
      resume_event_(base::WaitableEvent::ResetPolicy::AUTOMATIC,
                    base::WaitableEvent::InitialState::NOT_SIGNALED),
      output_(this),
      controller_(FROM_HERE) {}

TestWebosExtensionServerThread::~TestWebosExtensionServerThread() {
  if (client_)
    wl_client_destroy(client_);

  Resume();
  Stop();
}

bool TestWebosExtensionServerThread::Start() {
  display_.reset(wl_display_create());
  if (!display_)
    return false;
  event_loop_ = wl_display_get_event_loop(display_.get());

  int fd[2];
  if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, fd) < 0)
    return false;
  base::ScopedFD server_fd(fd[0]);
  base::ScopedFD client_fd(fd[1]);

  // If client has not specified rect before, user standard ones.
  if (output_.GetPhysicalSize().IsEmpty())
    output_.SetPhysicalAndLogicalBounds(gfx::Rect(0, 0, 800, 600));

  if (wl_display_init_shm(display_.get()) < 0)
    return false;
  if (!compositor_.Initialize(display_.get()))
    return false;
  if (!output_.Initialize(display_.get()))
    return false;
  if (!seat_.Initialize(display_.get()))
    return false;
  if (!shell_.Initialize(display_.get()))
    return false;
  if (!webos_shell_.Initialize(display_.get()))
    return false;

  client_ = wl_client_create(display_.get(), server_fd.release());
  if (!client_)
    return false;

  base::Thread::Options options;
  options.message_pump_factory =
      base::BindRepeating(&TestWebosExtensionServerThread::CreateMessagePump,
                          base::Unretained(this));
  if (!base::Thread::StartWithOptions(std::move(options)))
    return false;

  setenv(kWaylandEnvVarName, base::NumberToString(client_fd.release()).c_str(),
         1);

  return true;
}

void TestWebosExtensionServerThread::Pause() {
  task_runner()->PostTask(
      FROM_HERE, base::BindOnce(&TestWebosExtensionServerThread::DoPause,
                                base::Unretained(this)));
  pause_event_.Wait();
}

void TestWebosExtensionServerThread::Resume() {
  if (display_)
    wl_display_flush_clients(display_.get());
  resume_event_.Signal();
}

void TestWebosExtensionServerThread::OnTestOutputFlush(
    TestOutput* test_output,
    const TestOutputMetrics& metrics) {
  // Do nothing at the moment.
}

void TestWebosExtensionServerThread::OnTestOutputGlobalDestroy(
    TestOutput* test_output) {
  // Do nothing at the moment.
}


void TestWebosExtensionServerThread::DoPause() {
  base::RunLoop().RunUntilIdle();
  pause_event_.Signal();
  resume_event_.Wait();
}

std::unique_ptr<base::MessagePump>
TestWebosExtensionServerThread::CreateMessagePump() {
  auto pump = std::make_unique<base::MessagePumpLibevent>();
  pump->WatchFileDescriptor(wl_event_loop_get_fd(event_loop_), true,
                            base::MessagePumpLibevent::WATCH_READ, &controller_,
                            this);
  return std::move(pump);
}

void TestWebosExtensionServerThread::OnFileCanReadWithoutBlocking(int fd) {
  wl_event_loop_dispatch(event_loop_, 0);
  wl_display_flush_clients(display_.get());
}

void TestWebosExtensionServerThread::OnFileCanWriteWithoutBlocking(int fd) {}

}  // namespace wl
