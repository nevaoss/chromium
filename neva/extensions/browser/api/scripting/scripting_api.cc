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
// //chrome/browser/extensions/api/scripting/scripting_api.cc

// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "neva/extensions/browser/api/scripting/scripting_api.h"

#include <optional>

#include "base/json/json_writer.h"
#include "base/types/optional_util.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/web_contents.h"
#include "extensions/browser/extension_file_task_runner.h"
#include "extensions/browser/extension_system.h"
#include "extensions/browser/extension_util.h"
#include "extensions/browser/load_and_localize_file.h"
#include "extensions/browser/scripting_utils.h"
#include "extensions/browser/user_script_manager.h"
#include "extensions/common/api/scripts_internal/script_serialization.h"
#include "extensions/common/error_utils.h"
#include "extensions/common/manifest_constants.h"
#include "extensions/common/mojom/execution_world.mojom-shared.h"
#include "extensions/common/mojom/host_id.mojom.h"
#include "extensions/common/permissions/api_permission.h"
#include "extensions/common/permissions/permissions_data.h"
#include "extensions/common/utils/content_script_utils.h"
#include "extensions/common/utils/extension_types_utils.h"
#include "neva/extensions/browser/neva_extensions_service_factory.h"
#include "neva/extensions/browser/neva_extensions_service_impl.h"
#include "neva/extensions/browser/tab_helper.h"
#include "neva/extensions/browser/web_contents_map.h"

namespace neva {

namespace {

constexpr char kCouldNotLoadFileError[] = "Could not load file: '*'.";
constexpr char kDuplicateFileSpecifiedError[] =
    "Duplicate file specified: '*'.";
constexpr char kExactlyOneOfCssAndFilesError[] =
    "Exactly one of 'css' and 'files' must be specified.";

// Note: CSS always injects as soon as possible, so we default to
// document_start. Because of tab loading, there's no guarantee this will
// *actually* inject before page load, but it will at least inject "soon".
constexpr extensions::mojom::RunLocation kCSSRunLocation =
    extensions::mojom::RunLocation::kDocumentStart;

// Converts the given `style_origin` to a CSSOrigin.
extensions::mojom::CSSOrigin ConvertStyleOriginToCSSOrigin(
    extensions::api::scripting::StyleOrigin style_origin) {
  extensions::mojom::CSSOrigin css_origin =
      extensions::mojom::CSSOrigin::kAuthor;
  switch (style_origin) {
    case extensions::api::scripting::StyleOrigin::kNone:
    case extensions::api::scripting::StyleOrigin::kAuthor:
      css_origin = extensions::mojom::CSSOrigin::kAuthor;
      break;
    case extensions::api::scripting::StyleOrigin::kUser:
      css_origin = extensions::mojom::CSSOrigin::kUser;
      break;
  }

  return css_origin;
}

extensions::mojom::ExecutionWorld ConvertExecutionWorld(
    extensions::api::scripting::ExecutionWorld world) {
  extensions::mojom::ExecutionWorld execution_world =
      extensions::mojom::ExecutionWorld::kIsolated;
  switch (world) {
    case extensions::api::scripting::ExecutionWorld::kNone:
    case extensions::api::scripting::ExecutionWorld::kIsolated:
      break;  // Default to mojom::ExecutionWorld::kIsolated.
    case extensions::api::scripting::ExecutionWorld::kMain:
      execution_world = extensions::mojom::ExecutionWorld::kMain;
  }

  return execution_world;
}

std::string InjectionKeyForCode(const extensions::mojom::HostID& host_id,
                                const std::string& code) {
  return extensions::ScriptExecutor::GenerateInjectionKey(
      host_id, /*script_url=*/GURL(), code);
}

std::string InjectionKeyForFile(const extensions::mojom::HostID& host_id,
                                const GURL& resource_url) {
  return extensions::ScriptExecutor::GenerateInjectionKey(
      host_id, resource_url,
      /*code=*/std::string());
}

// Constructs an array of file sources from the read file `data`.
std::vector<InjectedFileSource> ConstructFileSources(
    std::vector<std::unique_ptr<std::string>> data,
    std::vector<std::string> file_names) {
  // Note: CHECK (and not DCHECK) because if it fails, we have an out-of-bounds
  // access.
  CHECK_EQ(data.size(), file_names.size());
  const size_t num_sources = data.size();
  std::vector<InjectedFileSource> sources;
  sources.reserve(num_sources);
  for (size_t i = 0; i < num_sources; ++i)
    sources.emplace_back(std::move(file_names[i]), std::move(data[i]));

  return sources;
}

std::vector<extensions::mojom::JSSourcePtr> FileSourcesToJSSources(
    const extensions::Extension& extension,
    std::vector<InjectedFileSource> file_sources) {
  std::vector<extensions::mojom::JSSourcePtr> js_sources;
  js_sources.reserve(file_sources.size());
  for (auto& file_source : file_sources) {
    js_sources.push_back(extensions::mojom::JSSource::New(
        std::move(*file_source.data),
        extension.GetResourceURL(file_source.file_name)));
  }

  return js_sources;
}

std::vector<extensions::mojom::CSSSourcePtr> FileSourcesToCSSSources(
    const extensions::Extension& extension,
    std::vector<InjectedFileSource> file_sources) {
  extensions::mojom::HostID host_id(
      extensions::mojom::HostID::HostType::kExtensions, extension.id());

  std::vector<extensions::mojom::CSSSourcePtr> css_sources;
  css_sources.reserve(file_sources.size());
  for (auto& file_source : file_sources) {
    css_sources.push_back(extensions::mojom::CSSSource::New(
        std::move(*file_source.data),
        InjectionKeyForFile(host_id,
                            extension.GetResourceURL(file_source.file_name))));
  }

  return css_sources;
}

// Checks `files` and populates `resources_out` with the appropriate extension
// resource. Returns true on success; on failure, populates `error_out`.
bool GetFileResources(const std::vector<std::string>& files,
                      const extensions::Extension& extension,
                      std::vector<extensions::ExtensionResource>* resources_out,
                      std::string* error_out) {
  if (files.empty()) {
    static constexpr char kAtLeastOneFileError[] =
        "At least one file must be specified.";
    *error_out = kAtLeastOneFileError;
    return false;
  }

  std::vector<extensions::ExtensionResource> resources;
  for (const auto& file : files) {
    extensions::ExtensionResource resource = extension.GetResource(file);
    if (resource.extension_root().empty() || resource.relative_path().empty()) {
      *error_out = extensions::ErrorUtils::FormatErrorMessage(
          kCouldNotLoadFileError, file);
      return false;
    }

    // extensions::ExtensionResource doesn't implement an operator==.
    auto existing = base::ranges::find_if(
        resources, [&resource](const extensions::ExtensionResource& other) {
          return resource.relative_path() == other.relative_path();
        });

    if (existing != resources.end()) {
      // Disallow duplicates. Note that we could allow this, if we wanted (and
      // there *might* be reason to with JS injection, to perform an operation
      // twice?). However, this matches content script behavior, and injecting
      // twice can be done by chaining calls to executeScript() / insertCSS().
      // This isn't a robust check, and could probably be circumvented by
      // passing two paths that look different but are the same - but in that
      // case, we just try to load and inject the script twice, which is
      // inefficient, but safe.
      *error_out = extensions::ErrorUtils::FormatErrorMessage(
          kDuplicateFileSpecifiedError, file);
      return false;
    }

    resources.push_back(std::move(resource));
  }

  resources_out->swap(resources);
  return true;
}

using ResourcesLoadedCallback =
    base::OnceCallback<void(std::vector<InjectedFileSource>,
                            std::optional<std::string>)>;

// Checks the loaded content of extension resources. Invokes `callback` with
// the constructed file sources on success or with an error on failure.
void CheckLoadedResources(std::vector<std::string> file_names,
                          ResourcesLoadedCallback callback,
                          std::vector<std::unique_ptr<std::string>> file_data,
                          std::optional<std::string> load_error) {
  if (load_error) {
    std::move(callback).Run({}, std::move(load_error));
    return;
  }

  std::vector<InjectedFileSource> file_sources =
      ConstructFileSources(std::move(file_data), std::move(file_names));

  for (const auto& source : file_sources) {
    DCHECK(source.data);
    // TODO(devlin): What necessitates this encoding requirement? Is it needed
    // for blink injection?
    if (!base::IsStringUTF8(*source.data)) {
      static constexpr char kBadFileEncodingError[] =
          "Could not load file '*'. It isn't UTF-8 encoded.";
      std::string error = extensions::ErrorUtils::FormatErrorMessage(
          kBadFileEncodingError, source.file_name);
      std::move(callback).Run({}, std::move(error));
      return;
    }
  }

  std::move(callback).Run(std::move(file_sources), std::nullopt);
}

// Checks the specified `files` for validity, and attempts to load and localize
// them, invoking `callback` with the result. Returns true on success; on
// failure, populates `error`.
bool CheckAndLoadFiles(std::vector<std::string> files,
                       const extensions::Extension& extension,
                       bool requires_localization,
                       ResourcesLoadedCallback callback,
                       std::string* error) {
  std::vector<extensions::ExtensionResource> resources;
  if (!GetFileResources(files, extension, &resources, error))
    return false;

  LoadAndLocalizeResources(
      extension, resources, requires_localization,
      extensions::script_parsing::GetMaxScriptLength(),
      base::BindOnce(&CheckLoadedResources, std::move(files),
                     std::move(callback)));
  return true;
}

// Returns an error message string for when an extension cannot access a page it
// is attempting to.
std::string GetCannotAccessPageErrorMessage(
    const extensions::PermissionsData& permissions,
    const GURL& url) {
  if (permissions.HasAPIPermission(extensions::mojom::APIPermissionID::kTab)) {
    return extensions::ErrorUtils::FormatErrorMessage(
        extensions::manifest_errors::kCannotAccessPageWithUrl, url.spec());
  }
  return extensions::manifest_errors::kCannotAccessPage;
}

// Returns true if the `permissions` allow for injection into the given `frame`.
// If false, populates `error`.
bool HasPermissionToInjectIntoFrame(
    const extensions::PermissionsData& permissions,
    int tab_id,
    content::RenderFrameHost* frame,
    std::string* error) {
  GURL committed_url = frame->GetLastCommittedURL();
  if (committed_url.is_empty()) {
    if (!frame->IsInPrimaryMainFrame()) {
      // We can't check the pending URL for subframes from the //chrome layer.
      // Assume the injection is allowed; the renderer has additional checks
      // later on.
      return true;
    }
    // Unknown URL, e.g. because no load was committed yet. In this case we look
    // for any pending entry on the NavigationController associated with the
    // WebContents for the frame.
    content::WebContents* web_contents =
        content::WebContents::FromRenderFrameHost(frame);
    content::NavigationEntry* pending_entry =
        web_contents->GetController().GetPendingEntry();
    if (!pending_entry) {
      *error = extensions::manifest_errors::kCannotAccessPage;
      return false;
    }
    GURL pending_url = pending_entry->GetURL();
    if (pending_url.SchemeIsHTTPOrHTTPS() &&
        !permissions.CanAccessPage(pending_url, tab_id, error)) {
      // This catches the majority of cases where an extension tried to inject
      // on a newly-created navigating tab, saving us a potentially-costly IPC
      // and, maybe, slightly reducing (but not by any stretch eliminating) an
      // attack surface.
      *error = GetCannotAccessPageErrorMessage(permissions, pending_url);
      return false;
    }

    // Otherwise allow for now. The renderer has additional checks and will
    // fail the injection if needed.
    return true;
  }

  // TODO(devlin): Add more schemes here, in line with
  // https://crbug.com/55084.
  if (committed_url.SchemeIs(url::kAboutScheme) ||
      committed_url.SchemeIs(url::kDataScheme)) {
    url::Origin origin = frame->GetLastCommittedOrigin();
    const url::SchemeHostPort& tuple_or_precursor_tuple =
        origin.GetTupleOrPrecursorTupleIfOpaque();
    if (!tuple_or_precursor_tuple.IsValid()) {
      *error = GetCannotAccessPageErrorMessage(permissions, committed_url);
      return false;
    }

    committed_url = tuple_or_precursor_tuple.GetURL();
  }

  return permissions.CanAccessPage(committed_url, tab_id, error);
}

// Collects the frames for injection. Method will return false if an error is
// encountered.
bool CollectFramesForInjection(
    const extensions::api::scripting::InjectionTarget& target,
    content::WebContents* tab,
    std::set<int>& frame_ids,
    std::set<content::RenderFrameHost*>& frames,
    std::string* error_out) {
  if (target.document_ids) {
    for (const auto& id : *target.document_ids) {
      extensions::ExtensionApiFrameIdMap::DocumentId document_id =
          extensions::ExtensionApiFrameIdMap::DocumentIdFromString(id);

      if (!document_id) {
        *error_out = base::StringPrintf("Invalid document id %s", id.c_str());
        return false;
      }

      content::RenderFrameHost* frame =
          extensions::ExtensionApiFrameIdMap::Get()
              ->GetRenderFrameHostByDocumentId(document_id);

      // If the frame was not found or it matched another tab reject this
      // request.
      if (!frame || content::WebContents::FromRenderFrameHost(frame) != tab) {
        *error_out =
            base::StringPrintf("No document with id %s in tab with id %d",
                               id.c_str(), target.tab_id);
        return false;
      }

      // Convert the documentId into a frameId since the content will be
      // injected synchronously.
      frame_ids.insert(extensions::ExtensionApiFrameIdMap::GetFrameId(frame));
      frames.insert(frame);
    }
  } else {
    if (target.frame_ids) {
      frame_ids.insert(target.frame_ids->begin(), target.frame_ids->end());
    } else {
      frame_ids.insert(extensions::ExtensionApiFrameIdMap::kTopFrameId);
    }

    for (int frame_id : frame_ids) {
      content::RenderFrameHost* frame =
          extensions::ExtensionApiFrameIdMap::GetRenderFrameHostById(tab,
                                                                     frame_id);
      if (!frame) {
        *error_out = base::StringPrintf("No frame with id %d in tab with id %d",
                                        frame_id, target.tab_id);
        return false;
      }
      frames.insert(frame);
    }
  }
  return true;
}

// Returns true if the `target` can be accessed with the given `permissions`.
// If the target can be accessed, populates `script_executor_out`,
// `frame_scope_out`, and `frame_ids_out` with the appropriate values;
// if the target cannot be accessed, populates `error_out`.
bool CanAccessTarget(const extensions::PermissionsData& permissions,
                     const extensions::api::scripting::InjectionTarget& target,
                     content::BrowserContext* browser_context,
                     bool include_incognito_information,
                     extensions::ScriptExecutor** script_executor_out,
                     extensions::ScriptExecutor::FrameScope* frame_scope_out,
                     std::set<int>* frame_ids_out,
                     std::string* error_out) {
  TabHelper* tab_helper =
      NevaExtensionsServiceFactory::GetService(browser_context)->GetTabHelper();
  if (!tab_helper) {
    LOG(ERROR) << __func__ << " tab_helper is null";
    return false;
  }

  content::WebContents* tab = tab_helper->GetWebContentsFromId(target.tab_id);
  if (!tab) {
    // TODO(devlin): Add a constant for this in a centrally-consumable location.
    *error_out = base::StringPrintf("No tab with id: %d", target.tab_id);
    return false;
  }

  if ((target.all_frames && *target.all_frames == true) &&
      (target.frame_ids || target.document_ids)) {
    *error_out =
        "Cannot specify 'allFrames' if either 'frameIds' or 'documentIds' is "
        "specified.";
    return false;
  }

  if (target.frame_ids && target.document_ids) {
    *error_out = "Cannot specify both 'frameIds' and 'documentIds'.";
    return false;
  }

  extensions::ScriptExecutor* script_executor =
      new extensions::ScriptExecutor(tab);
  DCHECK(script_executor);

  extensions::ScriptExecutor::FrameScope frame_scope =
      target.all_frames && *target.all_frames == true
          ? extensions::ScriptExecutor::INCLUDE_SUB_FRAMES
          : extensions::ScriptExecutor::SPECIFIED_FRAMES;

  std::set<int> frame_ids;
  std::set<content::RenderFrameHost*> frames;
  if (!CollectFramesForInjection(target, tab, frame_ids, frames, error_out)) {
    return false;
  }

  // TODO(devlin): If `allFrames` is true, we error out if the extension
  // doesn't have access to the top frame (even if it may inject in child
  // frames). This is inconsistent with content scripts (which can execute on
  // child frames), but consistent with the old tabs.executeScript() API.
  for (content::RenderFrameHost* frame : frames) {
    DCHECK_EQ(content::WebContents::FromRenderFrameHost(frame), tab);
    if (!HasPermissionToInjectIntoFrame(permissions, target.tab_id, frame,
                                        error_out)) {
      return false;
    }
  }

  *frame_ids_out = std::move(frame_ids);
  *frame_scope_out = frame_scope;
  *script_executor_out = script_executor;
  return true;
}

extensions::api::scripts_internal::SerializedUserScript
ConvertRegisteredContentScriptToSerializedUserScript(
    extensions::api::scripting::RegisteredContentScript content_script) {
  auto convert_source_files = [](std::vector<std::string> files) {
    std::vector<extensions::api::scripts_internal::ScriptSource> converted;
    converted.reserve(files.size());
    for (auto& file : files) {
      extensions::api::scripts_internal::ScriptSource converted_source;
      converted_source.file = std::move(file);
      converted.push_back(std::move(converted_source));
    }
    return converted;
  };

  auto convert_execution_world =
      [](extensions::api::scripting::ExecutionWorld world) {
        switch (world) {
          case extensions::api::scripting::ExecutionWorld::kNone:
          case extensions::api::scripting::ExecutionWorld::kIsolated:
            return extensions::api::extension_types::ExecutionWorld::kIsolated;
          case extensions::api::scripting::ExecutionWorld::kMain:
            return extensions::api::extension_types::ExecutionWorld::kMain;
        }
      };

  extensions::api::scripts_internal::SerializedUserScript serialized_script;
  serialized_script.source =
      extensions::api::scripts_internal::Source::kDynamicContentScript;

  // Note: IDs have already been prefixed appropriately.
  serialized_script.id = std::move(content_script.id);
  // Note: `matches` are guaranteed to be non-null.
  serialized_script.matches = std::move(*content_script.matches);
  serialized_script.exclude_matches = std::move(content_script.exclude_matches);
  if (content_script.css) {
    serialized_script.css =
        convert_source_files(std::move(*content_script.css));
  }
  if (content_script.js) {
    serialized_script.js = convert_source_files(std::move(*content_script.js));
  }
  serialized_script.all_frames = content_script.all_frames;
  serialized_script.match_origin_as_fallback =
      content_script.match_origin_as_fallback;
  serialized_script.run_at = content_script.run_at;
  serialized_script.world = convert_execution_world(content_script.world);

  return serialized_script;
}

std::unique_ptr<extensions::UserScript> ParseUserScript(
    content::BrowserContext* browser_context,
    const extensions::Extension& extension,
    extensions::api::scripting::RegisteredContentScript content_script,
    std::u16string* error) {
  extensions::api::scripts_internal::SerializedUserScript serialized_script =
      ConvertRegisteredContentScriptToSerializedUserScript(
          std::move(content_script));

  std::unique_ptr<extensions::UserScript> user_script =
      extensions::script_serialization::ParseSerializedUserScript(
          serialized_script, extension, error);
  if (!user_script) {
    return nullptr;  // Parsing failed.
  }

  // Post conversion validation and values.
  // TODO(https://crbug.com/1494155): See which of these can be moved into
  // script_serialization::ParseSerializedUserScript().
  if (!extensions::script_parsing::ValidateMatchOriginAsFallback(
          user_script->match_origin_as_fallback(), user_script->url_patterns(),
          error)) {
    return nullptr;
  }

  user_script->set_incognito_enabled(
      extensions::util::IsIncognitoEnabled(extension.id(), browser_context));

  return user_script;
}

}  // namespace

InjectedFileSource::InjectedFileSource(std::string file_name,
                                       std::unique_ptr<std::string> data)
    : file_name(std::move(file_name)), data(std::move(data)) {}
InjectedFileSource::InjectedFileSource(InjectedFileSource&&) = default;
InjectedFileSource::~InjectedFileSource() = default;

ScriptingExecuteScriptFunction::ScriptingExecuteScriptFunction() = default;
ScriptingExecuteScriptFunction::~ScriptingExecuteScriptFunction() = default;

ExtensionFunction::ResponseAction ScriptingExecuteScriptFunction::Run() {
  std::optional<extensions::api::scripting::ExecuteScript::Params> params =
      extensions::api::scripting::ExecuteScript::Params::Create(args());
  EXTENSION_FUNCTION_VALIDATE(params);
  injection_ = std::move(params->injection);

  // Silently alias `function` to `func` for backwards compatibility.
  // TODO(devlin): Remove this in M95.
  if (injection_.function) {
    if (injection_.func) {
      return RespondNow(
          Error("Both 'func' and 'function' were specified. "
                "Only 'func' should be used."));
    }
    injection_.func = std::move(injection_.function);
  }

  if ((injection_.files && injection_.func) ||
      (!injection_.files && !injection_.func)) {
    return RespondNow(
        Error("Exactly one of 'func' and 'files' must be specified"));
  }

  if (injection_.files) {
    if (injection_.args)
      return RespondNow(Error("'args' may not be used with file injections."));

    // JS files don't require localization.
    constexpr bool kRequiresLocalization = false;
    std::string error;
    if (!CheckAndLoadFiles(
            std::move(*injection_.files), *extension(), kRequiresLocalization,
            base::BindOnce(&ScriptingExecuteScriptFunction::DidLoadResources,
                           this),
            &error)) {
      return RespondNow(Error(std::move(error)));
    }
    return RespondLater();
  }

  DCHECK(injection_.func);

  // TODO(devlin): This (wrapping a function to create an IIFE) is pretty hacky,
  // and along with the JSON-serialization of the arguments to curry in.
  // Add support to the ScriptExecutor to better support this case.
  std::string args_expression;
  if (injection_.args) {
    std::vector<std::string> string_args;
    string_args.reserve(injection_.args->size());
    for (const auto& arg : *injection_.args) {
      std::string json;
      if (!base::JSONWriter::Write(arg, &json))
        return RespondNow(Error("Unserializable argument passed."));
      string_args.push_back(std::move(json));
    }
    args_expression = base::JoinString(string_args, ",");
  }

  std::string code_to_execute = base::StringPrintf(
      "(%s)(%s)", injection_.func->c_str(), args_expression.c_str());

  std::vector<extensions::mojom::JSSourcePtr> sources;
  sources.push_back(
      extensions::mojom::JSSource::New(std::move(code_to_execute), GURL()));

  std::string error;
  if (!Execute(std::move(sources), &error))
    return RespondNow(Error(std::move(error)));

  return RespondLater();
}

void ScriptingExecuteScriptFunction::DidLoadResources(
    std::vector<InjectedFileSource> file_sources,
    std::optional<std::string> load_error) {
  if (load_error) {
    Respond(Error(std::move(*load_error)));
    return;
  }

  DCHECK(!file_sources.empty());

  std::string error;
  if (!Execute(FileSourcesToJSSources(*extension(), std::move(file_sources)),
               &error)) {
    Respond(Error(std::move(error)));
  }
}

bool ScriptingExecuteScriptFunction::Execute(
    std::vector<extensions::mojom::JSSourcePtr> sources,
    std::string* error) {
  extensions::ScriptExecutor* script_executor = nullptr;
  extensions::ScriptExecutor::FrameScope frame_scope =
      extensions::ScriptExecutor::SPECIFIED_FRAMES;
  std::set<int> frame_ids;

  TabHelper* tab_helper =
      NevaExtensionsServiceFactory::GetService(browser_context())
          ->GetTabHelper();
  if (!tab_helper) {
    LOG(ERROR) << __func__ << " tab_helper is null";
    return false;
  }

  content::WebContents* web_contents =
      tab_helper->GetWebContentsFromId(injection_.target.tab_id);
  if (!web_contents) {
    LOG(ERROR) << __func__ << " web_contents is null";
    return false;
  }

  script_executor = new extensions::ScriptExecutor(web_contents);
  frame_ids.insert(web_contents->GetPrimaryMainFrame()->GetFrameTreeNodeId().value());

  extensions::mojom::ExecutionWorld execution_world =
      ConvertExecutionWorld(injection_.world);

  // Extensions can specify that the script should be injected "immediately".
  // In this case, we specify kDocumentStart as the injection time. Due to
  // inherent raciness between tab creation and load and this function
  // execution, there is no guarantee that it will actually happen at
  // document start, but the renderer will appropriately inject it
  // immediately if document start has already passed.
  extensions::mojom::RunLocation run_location =
      injection_.inject_immediately && *injection_.inject_immediately
          ? extensions::mojom::RunLocation::kDocumentStart
          : extensions::mojom::RunLocation::kDocumentIdle;
  script_executor->ExecuteScript(
      extensions::mojom::HostID(
          extensions::mojom::HostID::HostType::kExtensions, extension()->id()),
      extensions::mojom::CodeInjection::NewJs(
          extensions::mojom::JSInjection::New(
              std::move(sources), execution_world, /*world_id=*/std::nullopt,
              blink::mojom::WantResultOption::kWantResult,
              user_gesture()
                  ? blink::mojom::UserActivationOption::kActivate
                  : blink::mojom::UserActivationOption::kDoNotActivate,
              blink::mojom::PromiseResultOption::kAwait)),
      frame_scope, frame_ids, extensions::ScriptExecutor::MATCH_ABOUT_BLANK,
      run_location, extensions::ScriptExecutor::DEFAULT_PROCESS,
      /* webview_src */ GURL(),
      base::BindOnce(&ScriptingExecuteScriptFunction::OnScriptExecuted, this));

  return true;
}

void ScriptingExecuteScriptFunction::OnScriptExecuted(
    std::vector<extensions::ScriptExecutor::FrameResult> frame_results) {
  // If only a single frame was included and the injection failed, respond with
  // an error.
  if (frame_results.size() == 1 && !frame_results[0].error.empty()) {
    Respond(Error(std::move(frame_results[0].error)));
    return;
  }

  // Otherwise, respond successfully. We currently just skip over individual
  // frames that failed. In the future, we can bubble up these error messages
  // to the extension.
  std::vector<extensions::api::scripting::InjectionResult> injection_results;
  for (auto& result : frame_results) {
    if (!result.error.empty())
      continue;
    extensions::api::scripting::InjectionResult injection_result;
    injection_result.result = std::move(result.value);
    injection_result.frame_id = result.frame_id;
    if (result.document_id)
      injection_result.document_id = result.document_id.ToString();

    // Put the top frame first; otherwise, any order.
    if (result.frame_id == extensions::ExtensionApiFrameIdMap::kTopFrameId) {
      injection_results.insert(injection_results.begin(),
                               std::move(injection_result));
    } else {
      injection_results.push_back(std::move(injection_result));
    }
  }

  Respond(
      ArgumentList(extensions::api::scripting::ExecuteScript::Results::Create(
          injection_results)));
}

ScriptingInsertCSSFunction::ScriptingInsertCSSFunction() = default;
ScriptingInsertCSSFunction::~ScriptingInsertCSSFunction() = default;

ExtensionFunction::ResponseAction ScriptingInsertCSSFunction::Run() {
  std::optional<extensions::api::scripting::InsertCSS::Params> params =
      extensions::api::scripting::InsertCSS::Params::Create(args());
  EXTENSION_FUNCTION_VALIDATE(params);

  injection_ = std::move(params->injection);

  if ((injection_.files && injection_.css) ||
      (!injection_.files && !injection_.css)) {
    return RespondNow(Error(kExactlyOneOfCssAndFilesError));
  }

  if (injection_.files) {
    // CSS files require localization.
    constexpr bool kRequiresLocalization = true;
    std::string error;
    if (!CheckAndLoadFiles(
            std::move(*injection_.files), *extension(), kRequiresLocalization,
            base::BindOnce(&ScriptingInsertCSSFunction::DidLoadResources, this),
            &error)) {
      return RespondNow(Error(std::move(error)));
    }
    return RespondLater();
  }

  DCHECK(injection_.css);

  extensions::mojom::HostID host_id(
      extensions::mojom::HostID::HostType::kExtensions, extension()->id());

  std::vector<extensions::mojom::CSSSourcePtr> sources;
  sources.push_back(extensions::mojom::CSSSource::New(
      std::move(*injection_.css),
      InjectionKeyForCode(host_id, *injection_.css)));

  std::string error;
  if (!Execute(std::move(sources), &error)) {
    return RespondNow(Error(std::move(error)));
  }

  return RespondLater();
}

void ScriptingInsertCSSFunction::DidLoadResources(
    std::vector<InjectedFileSource> file_sources,
    std::optional<std::string> load_error) {
  if (load_error) {
    Respond(Error(std::move(*load_error)));
    return;
  }

  DCHECK(!file_sources.empty());
  std::vector<extensions::mojom::CSSSourcePtr> sources =
      FileSourcesToCSSSources(*extension(), std::move(file_sources));

  std::string error;
  if (!Execute(std::move(sources), &error)) {
    Respond(Error(std::move(error)));
  }
}

bool ScriptingInsertCSSFunction::Execute(
    std::vector<extensions::mojom::CSSSourcePtr> sources,
    std::string* error) {
  extensions::ScriptExecutor* script_executor = nullptr;
  extensions::ScriptExecutor::FrameScope frame_scope =
      extensions::ScriptExecutor::SPECIFIED_FRAMES;
  std::set<int> frame_ids;
  if (!CanAccessTarget(*extension()->permissions_data(), injection_.target,
                       browser_context(), include_incognito_information(),
                       &script_executor, &frame_scope, &frame_ids, error)) {
    return false;
  }
  DCHECK(script_executor);

  script_executor->ExecuteScript(
      extensions::mojom::HostID(
          extensions::mojom::HostID::HostType::kExtensions, extension()->id()),
      extensions::mojom::CodeInjection::NewCss(
          extensions::mojom::CSSInjection::New(
              std::move(sources),
              ConvertStyleOriginToCSSOrigin(injection_.origin),
              extensions::mojom::CSSInjection::Operation::kAdd)),
      frame_scope, frame_ids, extensions::ScriptExecutor::MATCH_ABOUT_BLANK,
      kCSSRunLocation, extensions::ScriptExecutor::DEFAULT_PROCESS,
      /* webview_src */ GURL(),
      base::BindOnce(&ScriptingInsertCSSFunction::OnCSSInserted, this));

  return true;
}

void ScriptingInsertCSSFunction::OnCSSInserted(
    std::vector<extensions::ScriptExecutor::FrameResult> results) {
  // If only a single frame was included and the injection failed, respond with
  // an error.
  if (results.size() == 1 && !results[0].error.empty()) {
    Respond(Error(std::move(results[0].error)));
    return;
  }

  Respond(NoArguments());
}

ScriptingRegisterContentScriptsFunction::
    ScriptingRegisterContentScriptsFunction() = default;
ScriptingRegisterContentScriptsFunction::
    ~ScriptingRegisterContentScriptsFunction() = default;

ExtensionFunction::ResponseAction
ScriptingRegisterContentScriptsFunction::Run() {
  std::optional<extensions::api::scripting::RegisterContentScripts::Params>
      params =
          extensions::api::scripting::RegisterContentScripts::Params::Create(
              args());
  EXTENSION_FUNCTION_VALIDATE(params);

  std::vector<extensions::api::scripting::RegisteredContentScript>& scripts =
      params->scripts;
  extensions::ExtensionUserScriptLoader* loader =
      extensions::ExtensionSystem::Get(browser_context())
          ->user_script_manager()
          ->GetUserScriptLoaderForExtension(extension()->id());

  // Create script ids for dynamic content scripts.
  std::string error;
  std::set<std::string> existing_script_ids = loader->GetDynamicScriptIDs(
      extensions::UserScript::Source::kDynamicContentScript);
  std::set<std::string> new_script_ids =
      extensions::scripting::CreateDynamicScriptIds(
          scripts, extensions::UserScript::Source::kDynamicContentScript,
          existing_script_ids, &error);

  if (!error.empty()) {
    CHECK(new_script_ids.empty());
    return RespondNow(Error(std::move(error)));
  }

  // Parse content scripts.
  std::u16string parse_error;
  extensions::UserScriptList parsed_scripts;
  std::set<std::string> persistent_script_ids;

  parsed_scripts.reserve(scripts.size());
  for (size_t i = 0; i < scripts.size(); ++i) {
    if (!scripts[i].matches) {
      std::string error_script_id =
          extensions::UserScript::TrimPrefixFromScriptID(scripts[i].id);
      return RespondNow(
          Error(base::StringPrintf("Script with ID '%s' must specify 'matches'",
                                   error_script_id.c_str())));
    }

    std::unique_ptr<extensions::UserScript> user_script = ParseUserScript(
        browser_context(), *extension(), std::move(scripts[i]), &parse_error);
    if (!user_script) {
      return RespondNow(Error(base::UTF16ToASCII(parse_error)));
    }

    // Scripts will persist across sessions by default.
    if (!scripts[i].persist_across_sessions ||
        *scripts[i].persist_across_sessions) {
      persistent_script_ids.insert(user_script->id());
    }
    parsed_scripts.push_back(std::move(user_script));
  }

  // Add new script IDs now in case another call with the same script IDs is
  // made immediately following this one.
  loader->AddPendingDynamicScriptIDs(std::move(new_script_ids));

  extensions::GetExtensionFileTaskRunner()->PostTaskAndReplyWithResult(
      FROM_HERE,
      base::BindOnce(&extensions::scripting::ValidateParsedScriptsOnFileThread,
                     extensions::script_parsing::GetSymlinkPolicy(extension()),
                     std::move(parsed_scripts)),
      base::BindOnce(&ScriptingRegisterContentScriptsFunction::
                         OnContentScriptFilesValidated,
                     this, std::move(persistent_script_ids)));

  // Balanced in `OnContentScriptFilesValidated()` or
  // `OnContentScriptsRegistered()`.
  AddRef();
  return RespondLater();
}

void ScriptingRegisterContentScriptsFunction::OnContentScriptFilesValidated(
    std::set<std::string> persistent_script_ids,
    extensions::scripting::ValidateScriptsResult result) {
  // We cannot proceed if the `browser_context` is not valid as the
  // `ExtensionSystem` will not exist.
  if (!browser_context()) {
    Release();  // Matches the `AddRef()` in `Run()`.
    return;
  }

  auto error = std::move(result.second);
  auto scripts = std::move(result.first);
  extensions::ExtensionUserScriptLoader* loader =
      extensions::ExtensionSystem::Get(browser_context())
          ->user_script_manager()
          ->GetUserScriptLoaderForExtension(extension()->id());

  if (error.has_value()) {
    std::set<std::string> ids_to_remove;
    for (const auto& script : scripts) {
      ids_to_remove.insert(script->id());
    }

    loader->RemovePendingDynamicScriptIDs(std::move(ids_to_remove));
    Respond(Error(std::move(*error)));
    Release();  // Matches the `AddRef()` in `Run()`.
    return;
  }

  loader->AddDynamicScripts(
      std::move(scripts), std::move(persistent_script_ids),
      base::BindOnce(
          &ScriptingRegisterContentScriptsFunction::OnContentScriptsRegistered,
          this));
}

void ScriptingRegisterContentScriptsFunction::OnContentScriptsRegistered(
    const std::optional<std::string>& error) {
  if (error.has_value()) {
    Respond(Error(std::move(*error)));
  } else {
    Respond(NoArguments());
  }
  Release();  // Matches the `AddRef()` in `Run()`.
}

}  // namespace neva
