// Copyright 2015 LG Electronics, Inc.
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

#include "neva/injection/renderer/injection_data_manager.h"

#include "base/json/json_reader.h"
#include "base/strings/string_number_conversions.h"
#include "base/values.h"

namespace injections {

InjectionDataManager::InjectionDataManager() {
}

InjectionDataManager::~InjectionDataManager() {
}

void InjectionDataManager::SetInjectionData(
    const std::string& key,
    const std::string& value) {
  injection_data_[key] = value;
}

bool InjectionDataManager::UpdateInjectionData(
    const std::string& key,
    const std::string& value) {
  auto it = injection_data_.find(key);
  if (it != injection_data_.end()) {
    it->second = value;
    return true;
  }
  return false;
}

bool InjectionDataManager::GetInjectionData(
    const std::string& key,
    std::string& value) const {
  auto it = injection_data_.find(key);
  if (it != injection_data_.end()) {
    value = it->second;
    return true;
  }
  return false;
}

void InjectionDataManager::Initialize(
    const std::string& json,
    const std::vector<std::string>& data_keys) {

  if (!injection_data_.empty())
    injection_data_.clear();

  std::optional<base::Value> root(base::JSONReader::Read(json));
  if (root && root->is_dict()) {
    const base::Value* value(nullptr);
    for (const std::string& key : data_keys) {
      value = root->GetDict().Find(key);
      if (value) {
        if (value->is_string()) {
          SetInjectionData(key, value->GetString());
        } else if (value->is_bool()) {
          SetInjectionData(key, value->GetBool() ? "true" : "false");
        } else if (value->is_int()) {
          SetInjectionData(key, base::NumberToString(value->GetInt()));
        } else if (value->is_double()) {
          SetInjectionData(key, base::NumberToString(value->GetDouble()));
        }
      }
    }
  }
}

}  // namespace injections
