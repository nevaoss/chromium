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

#ifndef NEVA_EXTENSIONS_BROWSER_NEVA_EXTENSIONS_API_CLIENT_H_
#define NEVA_EXTENSIONS_BROWSER_NEVA_EXTENSIONS_API_CLIENT_H_

#include "extensions/browser/api/extensions_api_client.h"

namespace neva {

class NevaExtensionsAPIClient : public extensions::ExtensionsAPIClient {
 public:
  NevaExtensionsAPIClient();
  NevaExtensionsAPIClient(const NevaExtensionsAPIClient&) = delete;
  NevaExtensionsAPIClient& operator=(const NevaExtensionsAPIClient&) = delete;
  ~NevaExtensionsAPIClient() override;

  // ExtensionsAPIClient implementation.
  extensions::MessagingDelegate* GetMessagingDelegate() override;
  extensions::ManagementAPIDelegate* CreateManagementAPIDelegate()
      const override;

 private:
  std::unique_ptr<extensions::MessagingDelegate> messaging_delegate_;
};

}  // namespace neva

#endif  // NEVA_EXTENSIONS_BROWSER_NEVA_EXTENSIONS_API_CLIENT_H_
