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

#include "ui/base/ime/neva/input_method_common.h"

namespace ui {

InputContentType GetInputContentTypeFromTextInputType(
    TextInputType text_input_type) {
  switch (text_input_type) {
    case TEXT_INPUT_TYPE_NONE:
      return InputContentType::kNone;
    case TEXT_INPUT_TYPE_TEXT:
      return InputContentType::kText;
    case TEXT_INPUT_TYPE_PASSWORD:
      return InputContentType::kPassword;
    case TEXT_INPUT_TYPE_SEARCH:
      return InputContentType::kSearch;
    case TEXT_INPUT_TYPE_EMAIL:
      return InputContentType::kEmail;
    case TEXT_INPUT_TYPE_NUMBER:
      return InputContentType::kNumber;
    case TEXT_INPUT_TYPE_TELEPHONE:
      return InputContentType::kTelephone;
    case TEXT_INPUT_TYPE_URL:
      return InputContentType::kUrl;
    case TEXT_INPUT_TYPE_DATE:
      return InputContentType::kDate;
    case TEXT_INPUT_TYPE_DATE_TIME:
      return InputContentType::kDateTime;
    case TEXT_INPUT_TYPE_DATE_TIME_LOCAL:
      return InputContentType::kDateTimeLocal;
    case TEXT_INPUT_TYPE_MONTH:
      return InputContentType::kMonth;
    case TEXT_INPUT_TYPE_TIME:
      return InputContentType::kTime;
    case TEXT_INPUT_TYPE_WEEK:
      return InputContentType::kWeek;
    case TEXT_INPUT_TYPE_TEXT_AREA:
      return InputContentType::kTextArea;
    case TEXT_INPUT_TYPE_CONTENT_EDITABLE:
      return InputContentType::kContentEditable;
    case TEXT_INPUT_TYPE_DATE_TIME_FIELD:
      return InputContentType::kDateTimeField;
    default:
      return InputContentType::kText;
  }
}

}  // namespace ui
