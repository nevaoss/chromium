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

#include "webos/browser_shell/browser_shell_webos_main_delegate.h"

#include <grp.h>
#include <pwd.h>
#include <unistd.h>

#include <optional>

#include "base/logging.h"
#include "base/path_service.h"
#include "build/build_config.h"
#include "content/public/common/content_switches.h"
#include "neva/logging.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/ui_base_paths.h"
#include "webos/common/webos_resource_delegate.h"
#include "webos/renderer/webos_content_renderer_client.h"

namespace {
const char kLocaleResourcesDirName[] = "neva_locales";
const char kResourcesFileName[] = "webos_content.pak";
}  // namespace

namespace webos {

BrowserShellWebOSMainDelegate::BrowserShellWebOSMainDelegate(
    content::MainFunctionParams parameters)
    : browser_shell::BrowserShellMainDelegate(std::move(parameters)) {}

BrowserShellWebOSMainDelegate::~BrowserShellWebOSMainDelegate() = default;

void BrowserShellWebOSMainDelegate::PreSandboxStartup() {
  base::FilePath pak_file;
#if defined(USE_CBE)
  bool r = base::PathService::Get(base::DIR_ASSETS, &pak_file);
#else
  bool r = base::PathService::Get(base::DIR_MODULE, &pak_file);
#endif  // defined(USE_CBE)
  NEVA_DCHECK(r);
  ui::ResourceBundle::InitSharedInstanceWithPakPath(
      pak_file.Append(FILE_PATH_LITERAL(kResourcesFileName)));

  base::PathService::Override(ui::DIR_LOCALES,
                              pak_file.AppendASCII(kLocaleResourcesDirName));
}

std::optional<int> BrowserShellWebOSMainDelegate::BasicStartupComplete() {
  if (!base::CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kProcessType)) {
    ChangeUserIDGroupID();
  }
  return browser_shell::BrowserShellMainDelegate::BasicStartupComplete();
}

content::ContentRendererClient*
BrowserShellWebOSMainDelegate::CreateContentRendererClient() {
  content_renderer_client_.reset(new WebOSContentRendererClient());
  return content_renderer_client_.get();
}

content::ContentClient*
BrowserShellWebOSMainDelegate::CreateContentClient() {
  content_client_ = std::make_unique<WebOSContentClient>();
  neva_app_runtime::SetAppRuntimeContentClient(content_client_.get());
  return content_client_.get();
}

#if BUILDFLAG(IS_WEBOS)
void BrowserShellWebOSMainDelegate::ChangeUserIDGroupID() {
  const char* uid = getenv("WAM_UID");
  const char* gid = getenv("WAM_GID");

  if (uid && gid) {
    struct passwd* pwd = getpwnam(uid);
    struct group* grp = getgrnam(gid);

    NEVA_DCHECK(pwd);
    NEVA_DCHECK(grp);

    int ret = -1;
    if (grp) {
      ret = setgid(grp->gr_gid);
      NEVA_DCHECK(ret == 0);
      ret = initgroups(uid, grp->gr_gid);
      NEVA_DCHECK(ret == 0);
    }

    if (pwd) {
      ret = setuid(pwd->pw_uid);
      NEVA_DCHECK(ret == 0);
      setenv("HOME", pwd->pw_dir, 1);
    }
  }
}
#endif  // BUILDFLAG(IS_WEBOS)

}  // namespace webos
