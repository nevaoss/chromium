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

// Based on
// //chrome/renderer/extensions/api/extension_hooks_delegate.h

// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NEVA_EXTENSIONS_RENDERER_API_EXTENSION_HOOKS_DELEGATE_H_
#define NEVA_EXTENSIONS_RENDERER_API_EXTENSION_HOOKS_DELEGATE_H_

#include "extensions/renderer/bindings/api_binding_hooks_delegate.h"
#include "extensions/renderer/bindings/api_signature.h"
#include "v8/include/v8.h"

namespace extensions {
class NativeRendererMessagingService;
class ScriptContext;
}  // namespace extensions

namespace neva {

using namespace extensions;

// The custom hooks for the chrome.extension API.
class ExtensionHooksDelegate : public APIBindingHooksDelegate {
 public:
  explicit ExtensionHooksDelegate(
      NativeRendererMessagingService* messaging_service);

  ExtensionHooksDelegate(const ExtensionHooksDelegate&) = delete;
  ExtensionHooksDelegate& operator=(const ExtensionHooksDelegate&) = delete;

  ~ExtensionHooksDelegate() override;

  // APIBindingHooksDelegate:
  void InitializeTemplate(v8::Isolate* isolate,
                          v8::Local<v8::ObjectTemplate> object_template,
                          const APITypeReferenceMap& type_refs) override;
};

}  // namespace neva

#endif  // NEVA_EXTENSIONS_RENDERER_API_EXTENSION_HOOKS_DELEGATE_H_
