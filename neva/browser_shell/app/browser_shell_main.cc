// Copyright 2021 LG Electronics, Inc.
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

#include "content/public/app/content_main.h"
#include "content/public/common/main_function_params.h"
#include "neva/browser_shell/app/browser_shell_main_delegate.h"
#include "neva/browser_shell/common/browser_shell_file_access_controller.h"

int main(int argc, const char** argv) {
  browser_shell::BrowserShellFileAccessController file_access_controller;
  neva_app_runtime::SetFileAccessController(&file_access_controller);

  base::CommandLine cmd_line(argc, argv);
  content::MainFunctionParams params(&cmd_line);
  browser_shell::BrowserShellMainDelegate main_delegate(std::move(params));
  content::ContentMainParams content_params(&main_delegate);
  content_params.argc = argc;
  content_params.argv = argv;

  return content::ContentMain(std::move(content_params));
}
