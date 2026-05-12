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

// Defines all Neva additional command-line switches used by ui/base.

#ifndef UI_BASE_UI_BASE_NEVA_SWITCHES_H_
#define UI_BASE_UI_BASE_NEVA_SWITCHES_H_

#include "base/component_export.h"

namespace switches {

COMPONENT_EXPORT(UI_BASE) extern const char kEnableNevaIme[];
COMPONENT_EXPORT(UI_BASE) extern const char kUseOzoneWaylandVkb[];
COMPONENT_EXPORT(UI_BASE) extern const char kOzoneWaylandUseXDGShell[];
COMPONENT_EXPORT(UI_BASE) extern const char kUseAglShell[];

}  // namespace switches

#endif  // UI_BASE_UI_BASE_NEVA_SWITCHES_H_
