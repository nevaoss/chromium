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

#ifndef NEVA_INJECTION_PUBLIC_RENDERER_WEBOSSERVICEBRIDGE_WEBAPI_H_
#define NEVA_INJECTION_PUBLIC_RENDERER_WEBOSSERVICEBRIDGE_WEBAPI_H_

#include <string>

#include "base/component_export.h"

namespace blink {
class WebLocalFrame;
}  // namespace blink

namespace injections {

class COMPONENT_EXPORT(INJECTION) WebOSServiceBridgeWebAPI {
 public:
  static void Install(blink::WebLocalFrame* frame, const std::string&);
  static void Uninstall(blink::WebLocalFrame* frame);
};

}  // namespace injections

#endif  // NEVA_INJECTION_PUBLIC_RENDERER_WEBOSSERVICEBRIDGE_WEBAPI_H_
