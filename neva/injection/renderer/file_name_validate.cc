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

#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/strings/string_util.h"

namespace injections {

std::string GetValidatedFilePath(
    const std::string& file, const std::string& folder) {
  std::string file_path = file;
  if (base::StartsWith(file, "file://", base::CompareCase::SENSITIVE)) {
    // Starts with "file://" then remove this, but the file path is absolute
    file_path.erase(file_path.begin(), file_path.begin() + 7);
  } else if (!base::StartsWith(file, "/", base::CompareCase::SENSITIVE)) {
    // Just use file path which is based on app folder path
    // make it absolute file path : concatenate folder path and file path
    file_path = folder + "/" + file;
  }

  // then the file_ is absolute file path
  base::FilePath appFolder = base::FilePath(folder);
  base::FilePath filePath = base::FilePath(file_path);
  if (appFolder.IsParent(filePath) && base::PathExists(filePath))
    return file_path;
  return std::string();
}

}  // namespace injections
