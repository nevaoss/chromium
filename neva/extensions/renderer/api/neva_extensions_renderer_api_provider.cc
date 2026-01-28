// Copyright 2024 LG Electronics, Inc.
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

#include "neva/extensions/renderer/api/neva_extensions_renderer_api_provider.h"

#include "extensions/renderer/bindings/api_bindings_system.h"
#include "extensions/renderer/native_extension_bindings_system.h"
#include "neva/extensions/renderer/api/extension_hooks_delegate.h"
#include "neva/extensions/renderer/api/tabs_hooks_delegate.h"

namespace neva {

void NevaExtensionsRendererAPIProvider::RegisterNativeHandlers(
    extensions::ModuleSystem* module_system,
    extensions::NativeExtensionBindingsSystem* bindings_system,
    extensions::V8SchemaRegistry* v8_schema_registry,
    extensions::ScriptContext* context) const {}

void NevaExtensionsRendererAPIProvider::AddBindingsSystemHooks(
    extensions::Dispatcher* dispatcher,
    extensions::NativeExtensionBindingsSystem* bindings_system) const {
  extensions::APIBindingsSystem* bindings = bindings_system->api_system();
  bindings->RegisterHooksDelegate("tabs",
                                  std::make_unique<TabsHooksDelegate>(
                                      bindings_system->messaging_service()));
  bindings->RegisterHooksDelegate("extension",
                                  std::make_unique<ExtensionHooksDelegate>(
                                      bindings_system->messaging_service()));
}

void NevaExtensionsRendererAPIProvider::PopulateSourceMap(
    extensions::ResourceBundleSourceMap* source_map) const {}
void NevaExtensionsRendererAPIProvider::EnableCustomElementAllowlist() const {}
void NevaExtensionsRendererAPIProvider::RequireWebViewModules(
    extensions::ScriptContext* context) const {}

}  // namespace neva
