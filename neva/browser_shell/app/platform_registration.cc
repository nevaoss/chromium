// Copyright 2022 LG Electronics, Inc.
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

#include "neva/browser_shell/app/platform_registration.h"

#include "base/command_line.h"
#include "base/logging.h"
#include "neva/app_runtime/app/app_runtime_page_contents.h"
#include "neva/app_runtime/app/app_runtime_page_view.h"
#include "neva/app_runtime/app/app_runtime_shell_window.h"
#include "neva/browser_shell/common/browser_shell_switches.h"
#include "neva/pal_service/pal_platform_factory.h"
#include "neva/pal_service/public/application_registrator_delegate.h"

namespace browser_shell {

PlatformRegistration::PlatformRegistration(
    neva_app_runtime::ShellWindow* main_window)
    : main_window_(main_window) {
  base::CommandLine* cmd = base::CommandLine::ForCurrentProcess();
  if (cmd->HasSwitch(switches::kShellAppId)) {
    delegate_ =
        pal::PlatformFactory::Get()->CreateApplicationRegistratorDelegate(
            cmd->GetSwitchValueASCII(switches::kShellAppId),
            base::BindRepeating(&PlatformRegistration::OnEvent,
                                base::Unretained(this)));
    if (!delegate_) {
      LOG(ERROR) << __func__ << "(): Application registration is not supported";
      return;
    }

    if (delegate_->GetStatus() !=
        pal::ApplicationRegistratorDelegate::Status::kSuccess) {
      LOG(ERROR) << __func__ << "(): Application "
                 << cmd->GetSwitchValueASCII(switches::kShellAppId)
                 << " was not registered.";
    }
  }
}

PlatformRegistration::~PlatformRegistration() = default;

void PlatformRegistration::OnEvent(const std::string& event,
                                   const std::string& options) {
  if (main_window_ && (event == "relaunch")) {
    // TODO(neva): Workaround params. in SetFullscreen()
    // For workaround, set param. fullscreen to true.
    // WaylandToplevelWindow only supports display::kInvalidDisplayId for
    // target_display_id. This needs to be updated if WaylandToplevelWindow
    // implementation is changed.
    main_window_->SetFullscreen(true, display::kInvalidDisplayId);

    if (!options.empty()) {
      std::string js_line = base::StringPrintf(R"JS(
          var e_tab_open = new CustomEvent("applicationRelaunchedByOS", %s);
          document.dispatchEvent(e_tab_open);
          )JS", options.c_str());

      if (main_window_->GetMainPageContents()) {
        main_window_->GetMainPageContents()->ExecuteJavaScriptInMainFrame(
            js_line);
      }
    }
  }
}

void PlatformRegistration::OnMainWindowClosing() {
  main_window_ = nullptr;
}

}  // namespace browser_shell
