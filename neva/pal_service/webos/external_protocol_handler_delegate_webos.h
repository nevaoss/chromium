// Copyright 2023 LG Electronics, Inc.
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

#ifndef NEVA_PAL_SERVICE_WEBOS_EXTERNAL_PROTOCOL_HANDLER_DELEGATE_WEBOS_H_
#define NEVA_PAL_SERVICE_WEBOS_EXTERNAL_PROTOCOL_HANDLER_DELEGATE_WEBOS_H_

#include "neva/pal_service/public/external_protocol_handler_delegate.h"

#include <memory>

namespace pal {
namespace luna {
class Client;
}

namespace webos {

class ExternalProtocolHandlerDelegateWebOS
    : public ExternalProtocolHandlerDelegate {
 public:
  ExternalProtocolHandlerDelegateWebOS();
  ~ExternalProtocolHandlerDelegateWebOS() override;
  ExternalProtocolHandlerDelegateWebOS(
      const ExternalProtocolHandlerDelegateWebOS&) = delete;
  ExternalProtocolHandlerDelegateWebOS& operator=(
      const ExternalProtocolHandlerDelegateWebOS&) = delete;

  void HandleExternalProtocol(const std::string& app_id,
                              const std::string& translated_url) override;

 private:
  void LaunchEnactBrowserWithUrl(const std::string& url);

  std::unique_ptr<luna::Client> luna_client_;
};

}  // namespace webos
}  // namespace pal

#endif  // NEVA_PAL_SERVICE_WEBOS_EXTERNAL_PROTOCOL_HANDLER_DELEGATE_WEBOS_H_
