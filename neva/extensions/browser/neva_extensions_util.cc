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

#include "neva/extensions/browser/neva_extensions_util.h"

#include "base/command_line.h"
#include "base/files/file_util.h"
#include "base/strings/string_tokenizer.h"
#include "extensions/common/switches.h"

namespace neva {

void LoadExtensionsFromCommandLine(
    neva::NevaExtensionSystem* extension_system) {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (!command_line->HasSwitch(extensions::switches::kLoadExtension))
    return;

  base::CommandLine::StringType path_list =
      command_line->GetSwitchValueNative(extensions::switches::kLoadExtension);

  base::StringTokenizerT<base::CommandLine::StringType,
                         base::CommandLine::StringType::const_iterator>
      tokenizer(path_list, FILE_PATH_LITERAL(","));
  while (tokenizer.GetNext()) {
    extension_system->LoadExtension(
        base::MakeAbsoluteFilePath(base::FilePath(tokenizer.token_piece())));
  }
}

}  // namespace neva
