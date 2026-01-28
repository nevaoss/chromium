// Copyright 2017 LG Electronics, Inc.
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

#include "neva/wam_demo/wam_demo_main_delegate.h"

#include <optional>

#include "base/memory/ptr_util.h"
#include "build/build_config.h"
#include "content/public/common/content_switches.h"
#include "neva/wam_demo/wam_demo_service.h"
#include "neva/wam_demo/wam_demo_switches.h"

#if BUILDFLAG(IS_WEBOS)
#include <grp.h>
#include <pwd.h>
#include <unistd.h>

#include "neva/logging.h"
#endif  // BUILDFLAG(IS_WEBOS)

namespace wam_demo {

WamDemoMainDelegate::WamDemoMainDelegate(content::MainFunctionParams parameters)
    : parameters_(std::move(parameters)) {}

WamDemoMainDelegate::~WamDemoMainDelegate() {}

std::optional<int> WamDemoMainDelegate::BasicStartupComplete() {
#if BUILDFLAG(IS_WEBOS)
  std::string process_type =
      parameters_.command_line->GetSwitchValueASCII(switches::kProcessType);
  if (process_type.empty()) {
    ChangeUserIDGroupID();
  }
#endif  // BUILDFLAG(IS_WEBOS)

  return AppRuntimeMainDelegate::BasicStartupComplete();
}

void WamDemoMainDelegate::PreMainMessageLoopRun() {
  if (parameters_.command_line->GetArgs().empty()) {
    service_ = std::make_unique<WamDemoService>(std::move(parameters_));
    return;
  }

  const std::string appname("cmdline.app");
  const std::string appurl = parameters_.command_line->GetArgs()[0];
  const bool fullscreen =
      parameters_.command_line->HasSwitch(switches::kFullscreen) ||
      parameters_.command_line->HasSwitch(switches::kStartFullscreen);
  const bool frameless = false;
  service_ = std::make_unique<WamDemoService>(std::move(parameters_));
  service_->LaunchApplicationFromCLI(appname, appurl, fullscreen, frameless);
}

#if BUILDFLAG(IS_WEBOS)
void WamDemoMainDelegate::ChangeUserIDGroupID() {
  LOG(INFO) << __func__;
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

}  // namespace wam_demo
