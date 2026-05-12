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

#ifndef NEVA_PAL_SERVICE_WEBOS_APPLICATION_REGISTRATOR_DELEGATE_WEBOS_H_
#define NEVA_PAL_SERVICE_WEBOS_APPLICATION_REGISTRATOR_DELEGATE_WEBOS_H_

#include "base/functional/callback.h"
#include "base/memory/weak_ptr.h"
#include "base/values.h"
#include "neva/pal_service/luna/luna_client.h"
#include "neva/pal_service/public/application_registrator_delegate.h"

#include <memory>
#include <string>

namespace pal {
namespace webos {

class ApplicationRegistratorDelegateWebOS
    : public ApplicationRegistratorDelegate {
 public:
  explicit ApplicationRegistratorDelegateWebOS(std::string application_name,
                                               RepeatingResponse callback);
  ApplicationRegistratorDelegateWebOS(
      const ApplicationRegistratorDelegateWebOS&) = delete;
  ApplicationRegistratorDelegateWebOS& operator=(
      const ApplicationRegistratorDelegateWebOS&) = delete;
  ~ApplicationRegistratorDelegateWebOS() override;

  Status GetStatus() const override;
  std::string GetApplicationName() const override;

 private:
  void OnResponse(pal::luna::Client::ResponseStatus,
                  unsigned,
                  const std::string& json);

  bool ParseRelaunchEvent(const base::Value::Dict& root_dict,
                          std::string* options);
  const std::string application_name_;
  RepeatingResponse callback_;
  Status status_ = Status::kNotInitialized;
  std::shared_ptr<luna::Client> luna_client_;
  base::WeakPtrFactory<ApplicationRegistratorDelegateWebOS> weak_factory_;
};

}  // namespace webos
}  // namespace pal

#endif  // NEVA_PAL_SERVICE_WEBOS_APPLICATION_REGISTRATOR_DELEGATE_WEBOS_H_
