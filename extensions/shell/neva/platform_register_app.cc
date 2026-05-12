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

#include "extensions/shell/neva/platform_register_app.h"

#include "base/command_line.h"
#include "content/public/browser/web_contents.h"
#include "extensions/common/switches.h"
#include "neva/pal_service/pal_platform_factory.h"
#include "neva/pal_service/public/application_registrator_delegate.h"
#include "ui/aura/window.h"
#include "ui/aura/window_tree_host.h"

PlatformRegisterApp::PlatformRegisterApp(content::WebContents* web_contents)
    : content::WebContentsUserData<PlatformRegisterApp>(*web_contents),
      content::WebContentsObserver(web_contents),
      weak_factory_(this) {
  base::CommandLine* cmd = base::CommandLine::ForCurrentProcess();
  if (cmd->HasSwitch(extensions::switches::kWebOSAppId)) {
    std::string name =
        cmd->GetSwitchValueASCII(extensions::switches::kWebOSAppId);
    delegate_ =
        pal::PlatformFactory::Get()->CreateApplicationRegistratorDelegate(
          std::move(name),
          base::BindRepeating(&PlatformRegisterApp::OnMessage,
                              weak_factory_.GetWeakPtr()));

    if (delegate_->GetStatus() !=
        pal::ApplicationRegistratorDelegate::Status::kSuccess)
      LOG(ERROR) << __func__ << "(): no webOS-application identifier specified";
  }
}

PlatformRegisterApp::~PlatformRegisterApp() = default;

void PlatformRegisterApp::OnMessage(const std::string& message,
                                    const std::string&) {
  content::WebContents* contents = web_contents();
  if (!contents)
    return;

  if (message == "relaunch") {
    aura::Window* top_window = contents->GetTopLevelNativeWindow();
    if (top_window && top_window->GetHost()) {
      // TODO(neva): Workaround params. in SetFullscreen()
      // For workaround, set param. fullscreen to true.
      // WaylandToplevelWindow only supports display::kInvalidDisplayId for
      // target_display_id. This needs to be updated if WaylandToplevelWindow
      // implementation is changed.
      top_window->GetHost()->SetFullscreen(true, display::kInvalidDisplayId);
    }
  }
}

WEB_CONTENTS_USER_DATA_KEY_IMPL(PlatformRegisterApp);
