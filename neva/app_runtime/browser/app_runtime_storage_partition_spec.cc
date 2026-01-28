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

#include "neva/app_runtime/browser/app_runtime_storage_partition_spec.h"

#include "base/strings/string_util.h"

namespace neva_app_runtime {

StoragePartitionInfo ParseStoragePartitionSpec(
    std::string_view partition_spec) {
  StoragePartitionInfo storage_partition_info;
  if (partition_spec.empty() || partition_spec == "default") {
    storage_partition_info.name = "";
    storage_partition_info.off_the_record = false;
  } else if (base::StartsWith(partition_spec,
                              "persist:", base::CompareCase::SENSITIVE)) {
    storage_partition_info.name = partition_spec.substr(8);
    storage_partition_info.off_the_record = false;
  } else if (base::StartsWith(partition_spec,
                              "guest:", base::CompareCase::SENSITIVE)) {
    // That's not real guest WebContents. Handling 'guest' prefix
    // this way is made to be compatible with Neva Browser.
    storage_partition_info.name = partition_spec.substr(6);
    storage_partition_info.off_the_record = true;
  } else {
    storage_partition_info.name = partition_spec;
    storage_partition_info.off_the_record = true;
  }
  return storage_partition_info;
}

}  // namespace neva_app_runtime
