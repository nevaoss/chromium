// Copyright 2020 LG Electronics, Inc.
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

#include "webos/application_installation_handler.h"

#include "base/lazy_instance.h"
#include "base/notreached.h"

static base::LazyInstance<webos::ApplicationInstallationHandler>::
    DestructorAtExit g_webos_app_installer = LAZY_INSTANCE_INITIALIZER;

namespace webos {

ApplicationInstallationHandler* ApplicationInstallationHandler::GetInstance() {
  return g_webos_app_installer.Pointer();
}

void ApplicationInstallationHandler::OnAppInstalled(const std::string& app_id) {
  NOTIMPLEMENTED();
}

void ApplicationInstallationHandler::OnAppRemoved(const std::string& app_id) {
  NOTIMPLEMENTED();
}

}  // namespace webos
