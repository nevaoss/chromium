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

#ifndef NEVA_PAL_SERVICE_WEBOS_LANGUAGE_TRACKER_DELEGATE_WEBOS_H_
#define NEVA_PAL_SERVICE_WEBOS_LANGUAGE_TRACKER_DELEGATE_WEBOS_H_

#include "base/functional/callback.h"
#include "base/memory/weak_ptr.h"
#include "neva/pal_service/luna/luna_client.h"
#include "neva/pal_service/public/language_tracker_delegate.h"

#include <memory>
#include <string>

namespace pal {
namespace webos {

class LanguageTrackerDelegateWebOS : public LanguageTrackerDelegate {
 public:
  explicit LanguageTrackerDelegateWebOS(RepeatingResponse callback);
  LanguageTrackerDelegateWebOS(const LanguageTrackerDelegateWebOS&) = delete;
  LanguageTrackerDelegateWebOS& operator=(const LanguageTrackerDelegateWebOS&) =
      delete;
  ~LanguageTrackerDelegateWebOS() override;

  Status GetStatus() const override;
  std::string GetLanguageString() const override;

 private:
  void OnResponse(luna::Client::ResponseStatus,
                  unsigned,
                  const std::string& json);
  std::string language_string_;
  RepeatingResponse subscription_callback_;
  Status status_ = Status::kNotInitialized;
  std::shared_ptr<luna::Client> luna_client_;
  base::WeakPtrFactory<LanguageTrackerDelegateWebOS> weak_factory_;
};

}  // namespace webos
}  // namespace pal

#endif  // NEVA_PAL_SERVICE_WEBOS_LANGUAGE_TRACKER_DELEGATE_WEBOS_H_
