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

#include "neva/pal_service/luna/luna_client.h"

#include "base/json/json_reader.h"
#include "base/values.h"
#include "neva/pal_service/luna/luna_client_impl.h"

namespace pal {
namespace luna {
namespace {

std::map<std::string, std::weak_ptr<Client>> g_clients_table;

void CleanClientsTable() {
  for (auto it = g_clients_table.begin(); it != g_clients_table.end();) {
    if (it->second.expired())
      it = g_clients_table.erase(it);
    else
      ++it;
  }
}

}  // namespace

Client::~Client() {}

std::unique_ptr<Client> CreateClient(const Client::Params& params) {
  return std::make_unique<ClientImpl>(params);
}

std::shared_ptr<Client> GetSharedClient(const Client::Params& params) {
  auto it = g_clients_table.find(params.name);
  if (it != g_clients_table.end()) {
    if (auto client = it->second.lock()) {
      if (client->GetAppId() == params.appid)
        return client;
      else
        return std::shared_ptr<Client>();
    }
  } else {
    // To avoid a large accumulation of expired weak pointers and string
    // keys to them.
    CleanClientsTable();
  }

  // We don't use make_shared to release memory allocated for ClientImpl
  // as soon as last related shared_ptr has been removed.
  std::shared_ptr<ClientImpl> new_client(new ClientImpl(params));
  g_clients_table[params.name] = std::weak_ptr<Client>(new_client);
  return new_client;
}

bool IsSubscription(const std::string& param) {
  std::optional<base::Value> json = base::JSONReader::Read(param);
  if (!json || !json->is_dict())
    return false;

  const base::Value::Dict& dict = json->GetDict();
  if (dict.FindBool("subscribe").value_or(false)) {
    return true;
  }

  return dict.FindBool("watch").value_or(false);
}

}  // namespace luna
}  // namespace pal
