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

// Based on
// //extensions/browser/api/scripting/scripting_api.h

// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NEVA_EXTENSIONS_BROWSER_API_SCRIPTING_SCRIPTING_API_H_
#define NEVA_EXTENSIONS_BROWSER_API_SCRIPTING_SCRIPTING_API_H_

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "extensions/browser/extension_function.h"
#include "extensions/browser/script_executor.h"
#include "extensions/browser/scripting_utils.h"
#include "extensions/common/api/scripting.h"
#include "extensions/common/mojom/code_injection.mojom.h"
#include "extensions/common/user_script.h"

namespace neva {

// A simple helper struct to represent a read file (either CSS or JS) to be
// injected.
struct InjectedFileSource {
  InjectedFileSource(std::string file_name, std::string data);
  InjectedFileSource(InjectedFileSource&&);
  ~InjectedFileSource();

  std::string file_name;
  std::string data;
};

class ScriptingExecuteScriptFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("scripting.executeScript", SCRIPTING_EXECUTESCRIPT)

  ScriptingExecuteScriptFunction();
  ScriptingExecuteScriptFunction(const ScriptingExecuteScriptFunction&) =
      delete;
  ScriptingExecuteScriptFunction& operator=(
      const ScriptingExecuteScriptFunction&) = delete;

  // ExtensionFunction:
  ResponseAction Run() override;

 private:
  ~ScriptingExecuteScriptFunction() override;

  // Called when the resource files to be injected has been loaded.
  void DidLoadResources(std::vector<InjectedFileSource> file_sources,
                        std::optional<std::string> load_error);

  // Triggers the execution of `sources` in the appropriate context.
  // Returns true on success; on failure, populates `error`.
  bool Execute(std::vector<extensions::mojom::JSSourcePtr> sources,
               std::string* error);

  // Invoked when script execution is complete.
  void OnScriptExecuted(
      std::vector<extensions::ScriptExecutor::FrameResult> frame_results);

  extensions::api::scripting::ScriptInjection injection_;
};

class ScriptingInsertCSSFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("scripting.insertCSS", SCRIPTING_INSERTCSS)

  ScriptingInsertCSSFunction();
  ScriptingInsertCSSFunction(const ScriptingInsertCSSFunction&) = delete;
  ScriptingInsertCSSFunction& operator=(const ScriptingInsertCSSFunction&) =
      delete;

  // ExtensionFunction:
  ResponseAction Run() override;

 private:
  ~ScriptingInsertCSSFunction() override;

  // Called when the resource files to be injected has been loaded.
  void DidLoadResources(std::vector<InjectedFileSource> file_sources,
                        std::optional<std::string> load_error);

  // Triggers the execution of `sources` in the appropriate context.
  // Returns true on success; on failure, populates `error`.
  bool Execute(std::vector<extensions::mojom::CSSSourcePtr> sources,
               std::string* error);

  // Called when the CSS insertion is complete.
  void OnCSSInserted(
      std::vector<extensions::ScriptExecutor::FrameResult> results);

  extensions::api::scripting::CSSInjection injection_;
};

class ScriptingRegisterContentScriptsFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("scripting.registerContentScripts",
                             SCRIPTING_REGISTERCONTENTSCRIPTS)

  ScriptingRegisterContentScriptsFunction();
  ScriptingRegisterContentScriptsFunction(
      const ScriptingRegisterContentScriptsFunction&) = delete;
  ScriptingRegisterContentScriptsFunction& operator=(
      const ScriptingRegisterContentScriptsFunction&) = delete;

  // ExtensionFunction:
  ResponseAction Run() override;

 private:
  ~ScriptingRegisterContentScriptsFunction() override;

  // Called when script files have been checked.
  void OnContentScriptFilesValidated(
      std::set<std::string> persistent_script_ids,
      extensions::scripting::ValidateScriptsResult result);

  // Called when content scripts have been registered.
  void OnContentScriptsRegistered(const std::optional<std::string>& error);
};

}  // namespace neva

#endif  // NEVA_EXTENSIONS_BROWSER_API_SCRIPTING_SCRIPTING_API_H_
