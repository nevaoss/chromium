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

#ifndef NEVA_PAL_SERVICE_LUNA_LUNA_CLIENT_H_
#define NEVA_PAL_SERVICE_LUNA_LUNA_CLIENT_H_

#include "base/functional/callback.h"
#include "base/functional/callback_helpers.h"

#include <memory>
#include <string>

namespace pal {
namespace luna {

class Client {
 public:
  struct Params {
    std::string name;
    std::string appid;
  };

  virtual ~Client();

  virtual bool IsInitialized() const = 0;

  virtual std::string GetName() const = 0;

  virtual std::string GetAppId() const = 0;

  enum class ResponseStatus : int32_t {
    SUCCESS,
    CANCELED,
    ERROR,
  };

  using OnceResponse = base::OnceCallback<
      void (ResponseStatus, unsigned token, const std::string&)>;
  virtual bool Call(std::string uri,
                    std::string param,
                    OnceResponse callback = base::DoNothing(),
                    std::string on_cancel_value = std::string(),
                    unsigned* token = nullptr) = 0;

  virtual bool CallFromApp(std::string uri,
                           std::string param,
                           std::string app_id,
                           OnceResponse callback = base::DoNothing(),
                           std::string on_cancel_value = std::string(),
                           unsigned* token = nullptr) = 0;

  virtual void Cancel(unsigned token) = 0;

  using RepeatingResponse = base::RepeatingCallback<
      void (ResponseStatus, unsigned token, const std::string&)>;
  virtual bool Subscribe(std::string uri,
                         std::string param,
                         RepeatingResponse callback,
                         std::string on_cancel_value = std::string(),
                         unsigned* token = nullptr) = 0;

  virtual bool SubscribeFromApp(std::string uri,
                                std::string param,
                                std::string app_id,
                                RepeatingResponse callback,
                                std::string on_cancel_value = std::string(),
                                unsigned* token = nullptr) = 0;

  virtual void Unsubscribe(unsigned token) = 0;
};

std::unique_ptr<Client> CreateClient(const Client::Params& params);

std::shared_ptr<Client> GetSharedClient(const Client::Params& params);

bool IsSubscription(const std::string& param);

}  // namespace luna
}  // namespace pal

#endif  // NEVA_PAL_SERVICE_LUNA_LUNA_CLIENT_H_
