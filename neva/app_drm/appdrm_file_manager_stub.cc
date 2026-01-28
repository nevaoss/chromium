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

#include "neva/app_drm/appdrm_common.h"

#include <tuple>

#include "mojo/public/cpp/system/file_data_source.h"

namespace app_drm {

AppDRMFileDataSource::AppDRMFileDataSource(base::File file,
                                           const base::FilePath& path)
    : file_data_source_(
          std::make_unique<mojo::FileDataSource>(std::move(file))) {
  std::ignore = appdrm_;
}

AppDRMFileDataSource::~AppDRMFileDataSource() {}

void AppDRMFileDataSource::SetRange(uint64_t start, uint64_t end) {
  return static_cast<mojo::FileDataSource*>(file_data_source_.get())
      ->SetRange(start, end);
}

uint64_t AppDRMFileDataSource::GetLength() const {
  return file_data_source_->GetLength();
}

mojo::DataPipeProducer::DataSource::ReadResult AppDRMFileDataSource::Read(
    uint64_t offset,
    base::span<char> buffer) {
  return file_data_source_->Read(offset, std::move(buffer));
}

AppDRMFileManagerWrapper::AppDRMFileManagerWrapper(const std::string url_str)
    : url_(url_str) {}

AppDRMFileManagerWrapper::~AppDRMFileManagerWrapper() {}

bool AppDRMFileManagerWrapper::Check() {
  return false;
}

const std::string AppDRMFileManagerWrapper::Decrypt() {
  return std::string("");
}

}  // namespace app_drm
