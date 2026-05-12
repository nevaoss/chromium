// Copyright 2016 LG Electronics, Inc.
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

#include "neva/app_runtime/app/app_runtime_main_delegate.h"

#include <optional>

#include "base/base_paths.h"
#include "base/command_line.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/run_loop.h"
#include "build/util/chromium_git_revision.h"
#include "components/services/heap_profiling/public/cpp/profiling_client.h"
#include "content/public/common/content_switches.h"
#include "neva/app_runtime/browser/app_runtime_browser_switches.h"
#include "neva/app_runtime/browser/app_runtime_content_browser_client.h"
#include "neva/app_runtime/common/app_runtime_file_access_controller.h"
#include "neva/app_runtime/public/app_runtime_switches.h"
#include "neva/app_runtime/renderer/app_runtime_content_renderer_client.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/ui_base_paths.h"

namespace {

bool SubprocessNeedsResourceBundle(const std::string& process_type) {
  return process_type == switches::kZygoteProcess ||
         process_type == switches::kPpapiPluginProcess ||
         process_type == switches::kGpuProcess ||
         process_type == switches::kRendererProcess ||
         process_type == switches::kUtilityProcess;
}

static neva_app_runtime::AppRuntimeContentClient*
    g_content_client = nullptr;

static neva_app_runtime::AppRuntimeFileAccessController*
    g_file_access_controller = nullptr;

struct BrowserClientTraits
    : public base::internal::DestructorAtExitLazyInstanceTraits<
          neva_app_runtime::AppRuntimeContentBrowserClient> {
  static neva_app_runtime::AppRuntimeContentBrowserClient* New(void* instance) {
    return new neva_app_runtime::AppRuntimeContentBrowserClient();
  }
};

base::LazyInstance<
    neva_app_runtime::AppRuntimeContentBrowserClient, BrowserClientTraits>
        g_app_runtime_content_browser_client = LAZY_INSTANCE_INITIALIZER;
base::LazyInstance<neva_app_runtime::AppRuntimeContentRendererClient>::DestructorAtExit
    g_app_runtime_content_renderer_client = LAZY_INSTANCE_INITIALIZER;

const char kLocaleResourcesDirName[] = "neva_locales";
const char kResourcesFileName[] = "app_runtime_content.pak";
}  // namespace

namespace neva_app_runtime {

void SetAppRuntimeContentClient(AppRuntimeContentClient* content_client) {
  g_content_client = content_client;
}

AppRuntimeContentClient* GetAppRuntimeContentClient() {
  return g_content_client;
}

AppRuntimeContentBrowserClient* GetAppRuntimeContentBrowserClient() {
  return g_app_runtime_content_browser_client.Pointer();
}

void SetFileAccessController(AppRuntimeFileAccessController* p) {
  g_file_access_controller = p;
}

const AppRuntimeFileAccessController* GetFileAccessController() {
  return g_file_access_controller;
}

AppRuntimeMainDelegate::AppRuntimeMainDelegate() {}

AppRuntimeMainDelegate::~AppRuntimeMainDelegate() {}

void AppRuntimeMainDelegate::PreSandboxStartup() {
  InitializeResourceBundle();
}

void AppRuntimeMainDelegate::ProcessExiting(const std::string& process_type) {
  if (SubprocessNeedsResourceBundle(process_type))
    ui::ResourceBundle::CleanupSharedInstance();
}

std::optional<int> AppRuntimeMainDelegate::BasicStartupComplete() {
  // The TLS slot used by the memlog allocator shim needs to be initialized
  // early to ensure that it gets assigned a low slot number. If it gets
  // initialized too late, the glibc TLS system will require a malloc call in
  // order to allocate storage for a higher slot number. Since malloc is hooked,
  // this causes re-entrancy into the allocator shim, while the TLS object is
  // partially-initialized, which the TLS object is supposed to protect again.
  heap_profiling::InitTLSSlot();

  logging::LoggingSettings settings;
  logging::InitLogging(settings);

  // By default PID, TID printing is enabled and timestamp, tickckount printing
  // is disabled. In such combination logs look like:
  // [<pid>:<tid>:<log_level>:<file_name>] <message>

  // If you need timestamps or tickcount for the debugging purpose then provide
  // corresponding command line argument.
  // For example, if |enable_timestamp| is true, logs will look like:
  // [<pid>:<tid>:MMDD/HHMMSS.Milliseconds:<log_level>:<file_name>] <message>

  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  bool enable_timestamp =
      command_line->HasSwitch(switches::kEnableTimestampLogging);
  bool enable_tickcount =
      command_line->HasSwitch(switches::kEnableTickcountLogging);

  logging::SetLogItems(true /* enable_process_id */,
                       true /* enable_thread_id */, enable_timestamp,
                       enable_tickcount);

  // The idea was seamlessly taken from //chrome/app/chrome_main_delegate.cc
  if (command_line->HasSwitch(kGitRevision)) {
    LOG(INFO) << "CHROMIUM_GIT_REVISION=" << CHROMIUM_GIT_REVISION;
    return 0;  // exit with a success error code.
  }

  return std::nullopt;
}

void AppRuntimeMainDelegate::PreMainMessageLoopRun() {
}

void AppRuntimeMainDelegate::WillRunMainMessageLoop(
    std::unique_ptr<base::RunLoop>& run_loop) {
}

void AppRuntimeMainDelegate::InitializeResourceBundle() {
  base::FilePath pak_file;
#if defined(USE_CBE)
  bool r = base::PathService::Get(base::DIR_ASSETS, &pak_file);
#else
  bool r = base::PathService::Get(base::DIR_MODULE, &pak_file);
#endif  // defined(USE_CBE)
  DCHECK(r);
  ui::ResourceBundle::InitSharedInstanceWithPakPath(
      pak_file.Append(FILE_PATH_LITERAL(kResourcesFileName)));

  base::PathService::Override(ui::DIR_LOCALES,
                              pak_file.AppendASCII(kLocaleResourcesDirName));
}

content::ContentBrowserClient*
AppRuntimeMainDelegate::CreateContentBrowserClient() {
  g_app_runtime_content_browser_client.Pointer()->SetBrowserExtraParts(this);
  return g_app_runtime_content_browser_client.Pointer();
}

content::ContentRendererClient*
AppRuntimeMainDelegate::CreateContentRendererClient() {
  return g_app_runtime_content_renderer_client.Pointer();
}

content::ContentClient*
AppRuntimeMainDelegate::CreateContentClient() {
  content_client_ = std::make_unique<AppRuntimeContentClient>();
  SetAppRuntimeContentClient(content_client_.get());
  return content_client_.get();
}

}  // namespace neva_app_runtime
