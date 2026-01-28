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

#ifndef NEVA_EXTENSIONS_RENDERER_API_NEVA_EXTENSIONS_RENDERER_API_PROVIDER_H_
#define NEVA_EXTENSIONS_RENDERER_API_NEVA_EXTENSIONS_RENDERER_API_PROVIDER_H_

#include "extensions/renderer/extensions_renderer_api_provider.h"

namespace extensions {
class Dispatcher;
class ModuleSystem;
class NativeExtensionBindingsSystem;
class ResourceBundleSourceMap;
class ScriptContext;
class V8SchemaRegistry;
}  // namespace extensions

namespace neva {

// Provides capabilities for extension APIs defined at the //neva layer.
class NevaExtensionsRendererAPIProvider
    : public extensions::ExtensionsRendererAPIProvider {
 public:
  NevaExtensionsRendererAPIProvider() = default;
  NevaExtensionsRendererAPIProvider(const NevaExtensionsRendererAPIProvider&) =
      delete;
  NevaExtensionsRendererAPIProvider& operator=(
      const NevaExtensionsRendererAPIProvider&) = delete;
  ~NevaExtensionsRendererAPIProvider() override = default;

  // extensions::ExtensionsRendererAPIProvider implementation
  void RegisterNativeHandlers(
      extensions::ModuleSystem* module_system,
      extensions::NativeExtensionBindingsSystem* bindings_system,
      extensions::V8SchemaRegistry* v8_schema_registry,
      extensions::ScriptContext* context) const override;
  void AddBindingsSystemHooks(extensions::Dispatcher* dispatcher,
                              extensions::NativeExtensionBindingsSystem*
                                  bindings_system) const override;
  void PopulateSourceMap(
      extensions::ResourceBundleSourceMap* source_map) const override;
  void EnableCustomElementAllowlist() const override;
  void RequireWebViewModules(extensions::ScriptContext* context) const override;
};

}  // namespace neva

#endif  // NEVA_EXTENSIONS_RENDERER_API_NEVA_EXTENSIONS_RENDERER_API_PROVIDER_H_
