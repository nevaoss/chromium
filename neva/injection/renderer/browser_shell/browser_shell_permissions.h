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

#ifndef NEVA_INJECTION_RENDERER_BROWSER_SHELL_BROWSER_SHELL_PERMISSIONS_H_
#define NEVA_INJECTION_RENDERER_BROWSER_SHELL_BROWSER_SHELL_PERMISSIONS_H_

#include <string>

#include "gin/object_template_builder.h"
#include "gin/wrappable.h"
#include "mojo/public/cpp/bindings/associated_receiver.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "neva/browser_shell/service/public/mojom/browser_shell_permissions.mojom.h"
#include "neva/browser_shell/service/public/mojom/browser_shell_service.mojom.h"
#include "neva/injection/renderer/browser_shell/browser_shell_permission_request.h"
#include "neva/injection/renderer/injection_events_emitter.h"
#include "v8/include/v8.h"

namespace injections {

class BrowserShellPermissions final
    : public gin::Wrappable<BrowserShellPermissions>,
      public InjectionEventsEmitter<BrowserShellPermissions>,
      public browser_shell::mojom::PermissionsClient,
      public PermissionRequest::Delegate {
 public:
  static constexpr gin::WrapperInfo kWrapperInfo = {
      {gin::kEmbedderNativeGin},
      gin::kBrowserShellPermissions};

  BrowserShellPermissions(
      v8::Isolate* isolate,
      mojo::Remote<browser_shell::mojom::Permissions> remote);
  BrowserShellPermissions(const BrowserShellPermissions&) = delete;
  BrowserShellPermissions& operator=(const BrowserShellPermissions&) = delete;
  ~BrowserShellPermissions() override;

  // ObjectTemplateBuilder::SetMethod does not support exposing inherited
  // methods. Such proxy-calling inherited methods is ugly but the easiest
  // workaround.
  void RunGetEventNames(gin::Arguments* args) const {
    InjectionEventsEmitter::GetEventNames(args);
  }

  void RunEmit(gin::Arguments* args) {
    InjectionEventsEmitter::Emit(args);
  }

  void RunAddEventListener(gin::Arguments* args) {
    InjectionEventsEmitter::AddEventListener(args);
  }

  void RunAddOnceEventListener(gin::Arguments* args) {
    InjectionEventsEmitter::AddOnceEventListener(args);
  }

  int RunGetListenerCount(const std::string& name) const {
    return InjectionEventsEmitter::GetListenerCount(name);
  }

  void RunRemoveEventListener(gin::Arguments* args) {
    InjectionEventsEmitter::RemoveEventListener(args);
  }

  void RunRemoveAllEventListeners(gin::Arguments* args) {
    InjectionEventsEmitter::RemoveAllEventListeners(args);
  }

  // Override browser_shell::mojom::PermissionsClient:
  void OnPermissionRequested(const std::string& host,
                             const std::vector<std::string>& types,
                             uint64_t request_id,
                             uint64_t page_id) override;

  // Override PermissionRequest::Delegate
  void AckPermission(PermissionRequest::Decision result,
                     uint64_t request_id,
                     uint64_t page_id) override;

  // gin::Wrappable : v8::Object::Wrappable
  void Trace(cppgc::Visitor* visitor) const final;

 private:
  // gin::WrappableBase
  const gin::WrapperInfo* wrapper_info() const override;

  // gin::Wrappable.
  gin::ObjectTemplateBuilder GetObjectTemplateBuilder(
      v8::Isolate* isolate) final;

  mojo::Remote<browser_shell::mojom::Permissions> remote_;
  mojo::AssociatedReceiver<browser_shell::mojom::PermissionsClient>
      receiver_{this};
};

}  // namespace injections

#endif  // NEVA_INJECTION_RENDERER_BROWSER_SHELL_BROWSER_SHELL_PERMISSIONS_H_
