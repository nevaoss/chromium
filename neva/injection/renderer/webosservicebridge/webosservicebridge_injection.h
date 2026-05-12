// Copyright 2019 LG Electronics, Inc.
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

#ifndef NEVA_INJECTION_RENDERER_WEBOSSERVICEBRIDGE_WEBOSSERVICEBRIDGE_INJECTION_H_
#define NEVA_INJECTION_RENDERER_WEBOSSERVICEBRIDGE_WEBOSSERVICEBRIDGE_INJECTION_H_

#include <memory>
#include <string>

#include "gin/object_template_builder.h"
#include "gin/wrappable.h"
#include "mojo/public/cpp/bindings/associated_receiver.h"
#include "mojo/public/cpp/bindings/pending_associated_receiver.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "neva/pal_service/public/mojom/system_servicebridge.mojom.h"
#include "v8/include/v8.h"

namespace blink {
class WebLocalFrame;
}

namespace gin {
class Arguments;
}

namespace injections {

class WebOSServiceBridgeProperties;

class WebOSServiceBridgeInjection
    : public gin::Wrappable<WebOSServiceBridgeInjection>,
      public pal::mojom::SystemServiceBridgeClient {
 public:
  static gin::WrapperInfo kWrapperInfo;

  static void Install(blink::WebLocalFrame* frame);
  static void Uninstall(blink::WebLocalFrame* frame);

  static bool HasWaitingRequests();
  static bool IsClosing();
  static void SetAppInClosing(bool closing);

  explicit WebOSServiceBridgeInjection(std::string appid);
  WebOSServiceBridgeInjection(const WebOSServiceBridgeInjection&) = delete;
  WebOSServiceBridgeInjection& operator=(const WebOSServiceBridgeInjection&) =
      delete;
  ~WebOSServiceBridgeInjection() override;

  // To handle luna call in webOSSystem.onclose callback
  static std::set<WebOSServiceBridgeInjection*> waiting_responses_;
  static bool is_closing_;

 private:
  static void WebOSServiceBridgeConstructorCallback(
      WebOSServiceBridgeProperties* properties,
      gin::Arguments* args);

  void Call(gin::Arguments* args);
  void DoCall(std::string uri, std::string payload);
  void Cancel();
  void DoNothing();

  bool IsWebOSSystemLoaded();
  void CloseNotify();
  void PreserveReferenceToInjectionObject(bool preserve);

  void OnConnect(
      mojo::PendingAssociatedReceiver<pal::mojom::SystemServiceBridgeClient>
          receiver);
  void CallJSHandler(const std::string& body);
  void Response(pal::mojom::ResponseStatus status,
                const std::string& payload) override;

  gin::ObjectTemplateBuilder GetObjectTemplateBuilder(
      v8::Isolate* isolate) override;

  std::string identifier_;
  bool subscribed_ = false;
  mojo::AssociatedReceiver<pal::mojom::SystemServiceBridgeClient>
      client_receiver_;
  mojo::Remote<pal::mojom::SystemServiceBridge> remote_system_bridge_;
};

}  // namespace injections

#endif  // NEVA_INJECTION_RENDERER_WEBOSSERVICEBRIDGE_WEBOSSERVICEBRIDGE_INJECTION_H_
