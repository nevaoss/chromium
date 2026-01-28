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
// //chrome/renderer/extensions/api/extension_hooks_delegate.cc

// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "neva/extensions/renderer/api/extension_hooks_delegate.h"

#include "extensions/renderer/api/messaging/native_renderer_messaging_service.h"
#include "extensions/renderer/extensions_renderer_client.h"

namespace neva {

using namespace extensions;

ExtensionHooksDelegate::ExtensionHooksDelegate(
    NativeRendererMessagingService* messaging_service) {}
ExtensionHooksDelegate::~ExtensionHooksDelegate() {}

void ExtensionHooksDelegate::InitializeTemplate(
    v8::Isolate* isolate,
    v8::Local<v8::ObjectTemplate> object_template,
    const APITypeReferenceMap& type_refs) {
  // TODO(neva): Current ExtensionsRendererClient::IsIncognitoProcess() returns
  // false only. If Neva browser supports separate service worker for private
  // browsing, implementation following the upstream logic could be possible.
  bool is_incognito = ExtensionsRendererClient::Get()->IsIncognitoProcess();
  object_template->Set(isolate, "inIncognitoContext",
                       v8::Boolean::New(isolate, is_incognito));
}

}  // namespace neva
