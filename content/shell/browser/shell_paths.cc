// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/shell/browser/shell_paths.h"

#include "base/base_paths.h"
#include "base/environment.h"
#include "base/files/file_util.h"
#include "base/path_service.h"
#include "base/threading/thread_restrictions.h"
#include "build/build_config.h"

#if BUILDFLAG(IS_FUCHSIA)
#include "base/fuchsia/file_utils.h"
#elif BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS)
#include "base/nix/xdg_util.h"
#endif

#if defined(USE_NEVA_APPRUNTIME)
#include "base/command_line.h"
#include "base/logging.h"
#include "content/shell/common/shell_neva_switches.h"
#endif

namespace content {

namespace {

bool GetDefaultUserDataDirectory(base::FilePath* result) {
#if BUILDFLAG(IS_WIN)
  CHECK(base::PathService::Get(base::DIR_LOCAL_APP_DATA, result));
  *result = result->Append(std::wstring(L"content_shell"));
#elif BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS)
  std::unique_ptr<base::Environment> env(base::Environment::Create());
  base::FilePath config_dir(base::nix::GetXDGDirectory(
      env.get(), base::nix::kXdgConfigHomeEnvVar, base::nix::kDotConfigDir));
  *result = config_dir.Append("content_shell");
#if defined(USE_NEVA_APPRUNTIME)
  // Overwrite user data dir value
  base::CommandLine* cmd_line = base::CommandLine::ForCurrentProcess();
  if (cmd_line->HasSwitch(switches::kUserDataDir)) {
    base::FilePath new_path =
        cmd_line->GetSwitchValuePath(switches::kUserDataDir);
    if (!new_path.empty()) {
      if (!new_path.IsAbsolute()) {
        LOG(INFO) << "kUserDataDir is not absolute path";
        base::FilePath current_dir;
        base::GetCurrentDirectory(&current_dir);
        *result = current_dir.Append(new_path);
      } else {
        *result = new_path;
      }
      LOG(INFO) << "kUserDataDir is set.";
    } else {
      LOG(INFO) << "kUserDataDir is empty.";
    }
  } else {
    LOG(INFO) << "kUserDataDir isn't set.";
  }
  LOG(INFO) << "Will use user data dir = " << result->value();
#endif  // defined(USE_NEVA_APPRUNTIME)
#elif BUILDFLAG(IS_APPLE)
  CHECK(base::PathService::Get(base::DIR_APP_DATA, result));
  *result = result->Append("Chromium Content Shell");
#elif BUILDFLAG(IS_ANDROID)
  CHECK(base::PathService::Get(base::DIR_ANDROID_APP_DATA, result));
  *result = result->Append(FILE_PATH_LITERAL("content_shell"));
#elif BUILDFLAG(IS_FUCHSIA)
  *result = base::FilePath(base::kPersistedDataDirectoryPath)
                .Append(FILE_PATH_LITERAL("content_shell"));
#else
  NOTIMPLEMENTED();
  return false;
#endif
  return true;
}

}  // namespace

class ShellPathProvider {
 public:
  static void CreateDir(const base::FilePath& path) {
    base::ScopedAllowBlocking allow_io;
    if (!base::PathExists(path))
      base::CreateDirectory(path);
  }
};

bool ShellPathProvider(int key, base::FilePath* result) {
  base::FilePath cur;

  switch (key) {
    case SHELL_DIR_USER_DATA: {
      bool rv = GetDefaultUserDataDirectory(result);
      if (rv)
        ShellPathProvider::CreateDir(*result);
      return rv;
    }
    default:
      return false;
  }
}

void RegisterShellPathProvider() {
  base::PathService::RegisterProvider(ShellPathProvider, SHELL_PATH_START,
                                      SHELL_PATH_END);
}

}  // namespace content
