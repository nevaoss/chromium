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

#include "webos/app/webos_content_main_delegate.h"

#include <optional>

#include "base/base_switches.h"
#include "base/command_line.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "content/public/common/content_switches.h"
#include "neva/app_runtime/browser/app_runtime_content_browser_client.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/ui_base_paths.h"
#include "webos/common/webos_resource_delegate.h"
#include "webos/renderer/webos_content_renderer_client.h"

using base::CommandLine;

namespace {
const char kLocaleResourcesDirName[] = "neva_locales";
const char kResourcesFileName[] = "webos_content.pak";
}  // namespace

namespace webos {

WebOSContentMainDelegate::WebOSContentMainDelegate() {}
WebOSContentMainDelegate::~WebOSContentMainDelegate() {}

std::optional<int> WebOSContentMainDelegate::BasicStartupComplete() {
  base::CommandLine* parsedCommandLine = base::CommandLine::ForCurrentProcess();

  if (basic_startup_callback_) {
    std::move(basic_startup_callback_).Run();
  }

  // TODO(pikulik): should be revised
  // parsedCommandLine->AppendSwitch(switches::kNoSandbox);

  std::string process_type =
        parsedCommandLine->GetSwitchValueASCII(switches::kProcessType);
  if (process_type.empty()) {
    std::move(startup_callback_).Run();
  }

  std::ignore = AppRuntimeMainDelegate::BasicStartupComplete();
  return std::nullopt;
}

void WebOSContentMainDelegate::PreSandboxStartup() {
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

content::ContentRendererClient*
WebOSContentMainDelegate::CreateContentRendererClient() {
#if !defined(USE_WEBOS_STARFISH) && !defined(USE_WEBOS_SIGNAGE)
  content_renderer_client_ =
      std::make_unique<WebOSContentRendererClient>(false);
#else
  content_renderer_client_ = std::make_unique<WebOSContentRendererClient>();
#endif
  return content_renderer_client_.get();
}

content::ContentClient*
WebOSContentMainDelegate::CreateContentClient() {
  content_client_ = std::make_unique<WebOSContentClient>();
  neva_app_runtime::SetAppRuntimeContentClient(content_client_.get());
  return content_client_.get();
}

}  // namespace webos
