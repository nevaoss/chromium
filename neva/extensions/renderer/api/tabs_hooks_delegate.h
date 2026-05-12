// Copyright 2023 LG Electronics, Inc.
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

#ifndef NEVA_EXTENSIONS_RENDERER_API_TABS_HOOKS_DELEGATE_H_
#define NEVA_EXTENSIONS_RENDERER_API_TABS_HOOKS_DELEGATE_H_

#include "base/memory/raw_ptr.h"
#include "extensions/renderer/bindings/api_binding_hooks_delegate.h"
#include "extensions/renderer/bindings/api_signature.h"
#include "v8/include/v8.h"

namespace extensions {
class NativeRendererMessagingService;
class ScriptContext;
}  // namespace extensions

namespace neva {

using namespace extensions;

// The custom hooks for the tabs API.
class TabsHooksDelegate : public APIBindingHooksDelegate {
 public:
  explicit TabsHooksDelegate(NativeRendererMessagingService* messaging_service);

  TabsHooksDelegate(const TabsHooksDelegate&) = delete;
  TabsHooksDelegate& operator=(const TabsHooksDelegate&) = delete;

  ~TabsHooksDelegate() override;

  // APIBindingHooksDelegate:
  APIBindingHooks::RequestResult HandleRequest(
      const std::string& method_name,
      const APISignature* signature,
      v8::Local<v8::Context> context,
      v8::LocalVector<v8::Value>* arguments,
      const APITypeReferenceMap& refs) override;

 private:
  // Request handlers for the corresponding API methods.
  APIBindingHooks::RequestResult HandleSendRequest(
      ScriptContext* script_context,
      const APISignature::V8ParseResult& parse_result);
  APIBindingHooks::RequestResult HandleSendMessage(
      ScriptContext* script_context,
      const APISignature::V8ParseResult& parse_result);
  APIBindingHooks::RequestResult HandleConnect(
      ScriptContext* script_context,
      const APISignature::V8ParseResult& parse_result);

  // The messaging service to handle connect() and sendMessage() calls.
  // Guaranteed to outlive this object.
  const raw_ptr<NativeRendererMessagingService> messaging_service_;
};

}  // namespace neva

#endif  // NEVA_EXTENSIONS_RENDERER_API_TABS_HOOKS_DELEGATE_H_
