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

#ifndef NEVA_INJECTION_RENDERER_BROWSER_SHELL_BROWSER_SHELL_IPC_INJECTION_H_
#define NEVA_INJECTION_RENDERER_BROWSER_SHELL_BROWSER_SHELL_IPC_INJECTION_H_

#include <string>

#include "gin/object_template_builder.h"
#include "gin/wrappable.h"
#include "mojo/public/cpp/bindings/associated_remote.h"
#include "mojo/public/cpp/bindings/associated_receiver.h"
#include "mojo/public/cpp/bindings/pending_associated_receiver.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "neva/browser_shell/service/public/mojom/browser_shell_ipc_endpoint.mojom.h"
#include "neva/browser_shell/service/public/mojom/browser_shell_service.mojom.h"
#include "neva/injection/renderer/injection_events_emitter.h"
#include "v8/include/v8.h"

namespace blink {
class WebLocalFrame;
}  // namespace blink

namespace injections {

class BrowserShellIpcInjection
    : public gin::Wrappable<BrowserShellIpcInjection>,
      public InjectionEventsEmitter<BrowserShellIpcInjection>,
      public browser_shell::mojom::ShellIpcEndpointClient {
 public:
  static gin::WrapperInfo kWrapperInfo;

  static const char kChannelPropertyName[];
  static const char kPostMethodName[];

  static void Install(blink::WebLocalFrame* frame);
  static void Uninstall(blink::WebLocalFrame* frame);

  static void ConstructorCallback(
      mojo::Remote<browser_shell::mojom::ShellService>* shell_service,
      gin::Arguments* args);

  BrowserShellIpcInjection(
      v8::Isolate* isolate,
      std::string channel,
      mojo::Remote<browser_shell::mojom::ShellIpcEndpoint> remote);
  BrowserShellIpcInjection(const BrowserShellIpcInjection&) = delete;
  BrowserShellIpcInjection& operator=(const BrowserShellIpcInjection&) = delete;
  ~BrowserShellIpcInjection() override;

  void Setup(
      mojo::PendingAssociatedReceiver<
          browser_shell::mojom::ShellIpcEndpointClient> receiver);

  const std::string& GetChannelName() const;
  void Post(gin::Arguments* args);

  // ObjectTemplateBuilder::SetMethod does not support exposing inherited
  // methods. Such proxy-calling inherited methods is ugly but the easiest
  // workaround.
  void RunGetEventNames(gin::Arguments* args) const {
    InjectionEventsEmitterBase::GetEventNames(args);
  }
  void RunEmit(gin::Arguments* args) { InjectionEventsEmitterBase::Emit(args); }
  void RunAddEventListener(gin::Arguments* args) {
    InjectionEventsEmitterBase::AddEventListener(args);
  }
  int RunGetListenerCount(const std::string& name) const {
    return InjectionEventsEmitterBase::GetListenerCount(name);
  }
  void RunAddOnceEventListener(gin::Arguments* args) {
    InjectionEventsEmitterBase::AddOnceEventListener(args);
  }
  void RunRemoveEventListener(gin::Arguments* args) {
    InjectionEventsEmitterBase::RemoveEventListener(args);
  }
  void RunRemoveAllEventListeners(gin::Arguments* args) {
    InjectionEventsEmitterBase::RemoveAllEventListeners(args);
  }

  // ShellIpcEndpointClient
  void Handle(const std::string& event, const std::string& json) override;

 private:
  const std::string channel_;
  mojo::Remote<browser_shell::mojom::ShellIpcEndpoint> remote_;
  mojo::AssociatedReceiver<browser_shell::mojom::ShellIpcEndpointClient>
      client_receiver_;

  gin::ObjectTemplateBuilder GetObjectTemplateBuilder(
      v8::Isolate* isolate) final;
};

}  // namespace injections

#endif  // NEVA_INJECTION_RENDERER_BROWSER_SHELL_BROWSER_SHELL_INJECTION_H_
