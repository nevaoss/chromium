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

#include "neva/browser_shell/common/browser_shell_switches.h"

namespace switches {

// Key to provide platform dependent Application Identificator.
const char kShellAppId[] = "shell-app-id";

// url to specify web-application running by browser-shell.
// Note that in case of local path file:// scheme should be specified.
const char kShellAppPath[] = "shell-app-path";

// Key to pass arguments called 'launch-args' to main Web application
// running by browser-shell.
const char kShellLaunchArgs[] = "launch-args";

// If specified ShellWindow starts in fullscreen mode.
const char kShellFullscreen[] = "shell-fullscreen";

// List for specify WEB API modules that will be connected to browser-shell.
const char kShellWebAPIs[] = "shell-web-apis";

}  // namespace switches
