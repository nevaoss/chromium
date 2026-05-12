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

#ifndef NEVA_INJECTION_RENDERER_WEBOSSERVICEBRIDGE_WEBOSSERVICEBRIDGE_PROPERTIES_H_
#define NEVA_INJECTION_RENDERER_WEBOSSERVICEBRIDGE_WEBOSSERVICEBRIDGE_PROPERTIES_H_

#include "base/memory/scoped_refptr.h"
#include "gin/arguments.h"
#include "neva/injection/renderer/injection_browser_control_base.h"
#include <string>

namespace blink {
class WebLocalFrame;
}

namespace injections {

class WebOSServiceBridgeProperties :
    public base::RefCounted<WebOSServiceBridgeProperties>,
    public InjectionBrowserControlBase {
 public:
  WebOSServiceBridgeProperties(blink::WebLocalFrame* frame);
  std::string GetIdentifier();

 private:
  std::string cached_identifier_;
};

}  // namespace injections

#endif  // NEVA_INJECTION_RENDERER_WEBOSSERVICEBRIDGE_WEBOSSERVICEBRIDGE_PROPERTIES_H_
