// Copyright 2018 LG Electronics, Inc.
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

#include "webos/renderer/webos_network_error_template_builder.h"

#include "base/json/json_string_value_serializer.h"
#include "base/logging.h"
#include "base/strings/string_util.h"
#include "ui/base/resource/resource_bundle.h"
#include "webos/grit/webos_network_error_resources.h"

using namespace ui;

namespace webos {

// Appends the source for i18n Templates in a script tag.
void AppendI18nTemplateSourceHtml(std::string* output) {
  const std::string err_page_template =
      ui::ResourceBundle::GetSharedInstance().LoadDataResourceString(
          IDR_WEBOS_NETWORK_ERROR_PAGE_TEMPLATE);
  if (err_page_template.empty()) {
    LOG(ERROR) << "Unable to get i18n template src";
    return;
  }

  output->append("<script>");
  output->append(err_page_template.data(), err_page_template.size());
  output->append("</script>");
}

void AppendJsonJS(const base::DictValue& json, std::string* output) {
  // Convert the template data to a json string.
  DCHECK(!json.empty()) << "must include json data structure";

  std::string jstext;
  JSONStringValueSerializer serializer(&jstext);
  serializer.Serialize(json);
  output->append("error_info = ");
  output->append(jstext);
  output->append(";");
}

void AppendJsonHtml(const base::DictValue& json, std::string* output) {
  std::string javascript_string;
  AppendJsonJS(json, &javascript_string);

  // </ confuses the HTML parser because it could be a </script> tag.  So we
  // replace </ with <\/.  The extra \ will be ignored by the JS engine.
  base::ReplaceSubstringsAfterOffset(&javascript_string, 0, "</", "<\\/");

  output->append("<script>");
  output->append(javascript_string);
  output->append("</script>");
}

std::string GetTemplatesHtml(const std::string_view& html_template,
                             const base::DictValue& json,
                             int err_code,
                             int viewport_width,
                             int viewport_height) {
  std::string output(html_template.data(), html_template.size());

  output.append("<script>");
  output.append("var errorCode = ");
  output.append(std::to_string(err_code));
  output.append(";");

  output.append("var appResolutionWidth = ");
  output.append(std::to_string(viewport_width));
  output.append(";");

  output.append("var appResolutionHeight = ");
  output.append(std::to_string(viewport_height));
  output.append(";");
  output.append("</script>");

  AppendJsonHtml(json, &output);
  AppendI18nTemplateSourceHtml(&output);

  return output;
}

}  // namespace webos
