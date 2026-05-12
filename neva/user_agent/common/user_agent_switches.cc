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

#include "neva/user_agent/common/user_agent_switches.h"

namespace neva_user_agent {

// Enable user agent client hints.
const char kEnableNevaUserAgentClientHints[] =
    "enable-neva-user-agent-client-hints";

// A string used to override the default user agent with a custom one.
const char kUserAgent[] = "user-agent";

}  // namespace neva_user_agent
