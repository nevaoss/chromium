// Copyright 2017-2019 LG Electronics, Inc.
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

#include "ui/base/ui_base_neva_switches.h"

namespace switches {

// Enables using of neva ime.
const char kEnableNevaIme[] = "enable-neva-ime";
const char kUseOzoneWaylandVkb[] = "use-ozone-wayland-vkb";
const char kOzoneWaylandUseXDGShell[] = "ozone-wayland-use-xdg-shell";

// Use agl_shell wayland compositor interface. It seems like AGL compositor
// supports the only client to bind to it and throws error on subsequent
// attempts, so use of agl_shell is disabled by default.
const char kUseAglShell[] = "use-agl-shell";

}  // namespace switches
