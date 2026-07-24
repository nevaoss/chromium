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
//
// This file is based on shell/shell/browser/net/proxying_url_loader_factory.h
// from ElectronJS project.
//
// Copyright (c) 2019 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#ifndef NEVA_APP_RUNTIME_BROWSER_NET_APP_RUNTIME_PROXYING_URL_LOADER_FACTORY_H_
#define NEVA_APP_RUNTIME_BROWSER_NET_APP_RUNTIME_PROXYING_URL_LOADER_FACTORY_H_

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "base/memory/weak_ptr.h"
#include "content/public/browser/content_browser_client.h"
#include "content/public/browser/render_frame_host.h"
#include "extensions/browser/api/web_request/web_request_info.h"
#include "ipc/constants.mojom.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "mojo/public/cpp/bindings/receiver_set.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "net/base/completion_once_callback.h"
#include "net/ssl/ssl_info.h"
#include "net/traffic_annotation/network_traffic_annotation.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/mojom/network_context.mojom.h"
#include "services/network/public/mojom/url_loader.mojom.h"
#include "services/network/public/mojom/url_loader_factory.mojom.h"
#include "services/network/public/mojom/url_response_head.mojom.h"
#include "services/network/url_loader_factory.h"
#include "url/gurl.h"

namespace neva_app_runtime {

class AppRuntimeWebRequestListener;

// AppRuntimeProxyingURLLoaderFactory is based on the ElectronJS's class
// electron::ProxyingURLLoaderFactory which originates from Chromium's
// extensions::WebRequestProxyingURLLoaderFactory

class AppRuntimeProxyingURLLoaderFactory
    : public network::mojom::URLLoaderFactory,
      public network::mojom::TrustedURLLoaderHeaderClient {
 public:
  class InProgressRequest : public network::mojom::URLLoader,
                            public network::mojom::URLLoaderClient,
                            public network::mojom::TrustedHeaderClient {
   public:
    // For usual requests
    InProgressRequest(
        AppRuntimeProxyingURLLoaderFactory* factory,
        uint64_t web_request_id,
        int32_t frame_routing_id,
        int32_t network_service_request_id,
        uint32_t options,
        const network::ResourceRequest& request,
        const net::MutableNetworkTrafficAnnotationTag& traffic_annotation,
        mojo::PendingReceiver<network::mojom::URLLoader> loader_receiver,
        mojo::PendingRemote<network::mojom::URLLoaderClient> client);
    // For CORS preflights
    InProgressRequest(AppRuntimeProxyingURLLoaderFactory* factory,
                      uint64_t request_id,
                      int32_t frame_routing_id,
                      const network::ResourceRequest& request);
    ~InProgressRequest() override;

    // Disable copy
    InProgressRequest(const InProgressRequest&) = delete;
    InProgressRequest& operator=(const InProgressRequest&) = delete;

    void Restart();

    // network::mojom::URLLoader:
    void FollowRedirect(
        network::HttpRequestHeadersUpdateParams headers_update_params,
        const std::optional<GURL>& new_url) override;
    void SetPriority(net::RequestPriority priority,
                     int32_t intra_priority_value) override;

    // network::mojom::URLLoaderClient:
    void OnReceiveEarlyHints(
        network::mojom::EarlyHintsPtr early_hints) override;
    void OnReceiveResponse(
        network::mojom::URLResponseHeadPtr head,
        mojo::ScopedDataPipeConsumerHandle body,
        std::optional<mojo_base::BigBuffer> cached_metadata) override;
    void OnReceiveRedirect(const net::RedirectInfo& redirect_info,
                           network::mojom::URLResponseHeadPtr head) override;
    void OnUploadProgress(int64_t current_position,
                          int64_t total_size,
                          OnUploadProgressCallback callback) override;
    void OnTransferSizeUpdated(int32_t transfer_size_diff) override;
    void OnComplete(const network::URLLoaderCompletionStatus& status) override;

    void OnLoaderCreated(
        mojo::PendingReceiver<network::mojom::TrustedHeaderClient> receiver);

    // network::mojom::TrustedHeaderClient:
    void OnBeforeSendHeaders(const GURL& request_url,
                             const net::HttpRequestHeaders& headers,
                             OnBeforeSendHeadersCallback callback) override;
    void OnHeadersReceived(const std::string& headers,
                           const net::IPEndPoint& endpoint,
                           const std::optional<net::SSLInfo>& ssl_info,
                           OnHeadersReceivedCallback callback) override;

   private:
    // These two methods combined form the implementation of Restart().
    void UpdateRequestInfo();
    void RestartInternal();

    void ContinueToBeforeSendHeaders(int error_code);
    void ContinueToSendHeaders(const std::set<std::string>& removed_headers,
                               const std::set<std::string>& set_headers,
                               int error_code);
    void ContinueToStartRequest(int error_code);
    void ContinueToHandleOverrideHeaders(int error_code);
    void ContinueToResponseStarted(int error_code);
    void ContinueToBeforeRedirect(const net::RedirectInfo& redirect_info,
                                  int error_code);
    void HandleResponseOrRedirectHeaders(
        net::CompletionOnceCallback continuation);
    void OnRequestError(const network::URLLoaderCompletionStatus& status);
    void HandleBeforeRequestRedirect();

    raw_ptr<AppRuntimeProxyingURLLoaderFactory> const factory_;
    network::ResourceRequest request_;
    const std::optional<url::Origin> original_initiator_;
    const uint64_t request_id_ = 0;
    const int32_t network_service_request_id_ = 0;
    const int32_t frame_routing_id_ = IPC::mojom::kRoutingIdNone;
    const uint32_t options_ = 0;
    const net::MutableNetworkTrafficAnnotationTag traffic_annotation_;
    mojo::Receiver<network::mojom::URLLoader> proxied_loader_receiver_;
    mojo::Remote<network::mojom::URLLoaderClient> target_client_;

    std::optional<extensions::WebRequestInfo> info_;

    mojo::Receiver<network::mojom::URLLoaderClient> proxied_client_receiver_{
        this};
    mojo::Remote<network::mojom::URLLoader> target_loader_;

    network::mojom::URLResponseHeadPtr current_response_;
    mojo::ScopedDataPipeConsumerHandle current_body_;
    scoped_refptr<net::HttpResponseHeaders> override_headers_;
    GURL redirect_url_;

    const bool for_cors_preflight_ = false;

    // If |has_any_extra_headers_listeners_| is set to true, the request will be
    // sent with the network::mojom::kURLLoadOptionUseHeaderClient option, and
    // we expect events to come through the
    // network::mojom::TrustedURLLoaderHeaderClient binding on the factory. This
    // is only set to true if there is a listener that needs to view or modify
    // headers set in the network process.
    bool has_any_extra_headers_listeners_ = false;
    bool current_request_uses_header_client_ = false;
    OnBeforeSendHeadersCallback on_before_send_headers_callback_;
    OnHeadersReceivedCallback on_headers_received_callback_;
    mojo::Receiver<network::mojom::TrustedHeaderClient> header_client_receiver_{
        this};

    // If |has_any_extra_headers_listeners_| is set to false and a redirect is
    // in progress, this stores the parameters to FollowRedirect that came from
    // the client. That way we can combine it with any other changes that
    // extensions made to headers in their callbacks.
    struct FollowRedirectParams {
      FollowRedirectParams();
      ~FollowRedirectParams();
      network::HttpRequestHeadersUpdateParams headers_update_params;
      std::optional<GURL> new_url;

      // disable copy
      FollowRedirectParams(const FollowRedirectParams&) = delete;
      FollowRedirectParams& operator=(const FollowRedirectParams&) = delete;
    };
    std::unique_ptr<FollowRedirectParams> pending_follow_redirect_params_;
    std::optional<mojo_base::BigBuffer> current_cached_metadata_;
    base::WeakPtrFactory<InProgressRequest> weak_factory_{this};
  };

  AppRuntimeProxyingURLLoaderFactory(
      AppRuntimeWebRequestListener* web_request_listener,
      int render_process_id,
      int frame_routing_id,
      uint64_t* request_id_generator,
      std::unique_ptr<extensions::ExtensionNavigationUIData> navigation_ui_data,
      std::optional<int64_t> navigation_id,
      mojo::PendingReceiver<network::mojom::URLLoaderFactory> loader_request,
      mojo::PendingRemote<network::mojom::URLLoaderFactory>
          target_factory_remote,
      mojo::PendingReceiver<network::mojom::TrustedURLLoaderHeaderClient>
          header_client_receiver,
      content::ContentBrowserClient::URLLoaderFactoryType loader_factory_type);

  ~AppRuntimeProxyingURLLoaderFactory() override;

  // Disable copy
  AppRuntimeProxyingURLLoaderFactory(
      const AppRuntimeProxyingURLLoaderFactory&) = delete;
  AppRuntimeProxyingURLLoaderFactory& operator=(
      const AppRuntimeProxyingURLLoaderFactory&) = delete;

  // network::mojom::URLLoaderFactory:
  void CreateLoaderAndStart(
      mojo::PendingReceiver<network::mojom::URLLoader> loader,
      int32_t request_id,
      uint32_t options,
      const network::ResourceRequest& request,
      mojo::PendingRemote<network::mojom::URLLoaderClient> client,
      const net::MutableNetworkTrafficAnnotationTag& traffic_annotation)
      override;
  void Clone(mojo::PendingReceiver<network::mojom::URLLoaderFactory>
                 loader_receiver) override;

  // network::mojom::TrustedURLLoaderHeaderClient:
  void OnLoaderCreated(
      int32_t request_id,
      mojo::PendingReceiver<network::mojom::TrustedHeaderClient> receiver)
      override;
  void OnLoaderForCorsPreflightCreated(
      const network::ResourceRequest& request,
      mojo::PendingReceiver<network::mojom::TrustedHeaderClient> receiver)
      override;

  AppRuntimeWebRequestListener* web_request_listener() {
    return web_request_listener_;
  }

  bool IsForServiceWorkerScript() const;

 private:
  void OnTargetFactoryError();
  void OnProxyBindingError();
  void RemoveRequest(int32_t network_service_request_id, uint64_t request_id);
  void MaybeDeleteThis();

  bool ShouldIgnoreConnectionsLimit(const network::ResourceRequest& request);

  raw_ptr<AppRuntimeWebRequestListener> web_request_listener_;

  const int render_process_id_;
  const int frame_routing_id_;
  raw_ptr<uint64_t> request_id_generator_;  // managed by ElectronBrowserClient
  std::unique_ptr<extensions::ExtensionNavigationUIData> navigation_ui_data_;
  std::optional<int64_t> navigation_id_;
  mojo::ReceiverSet<network::mojom::URLLoaderFactory> proxy_receivers_;
  mojo::Remote<network::mojom::URLLoaderFactory> target_factory_;
  mojo::Receiver<network::mojom::TrustedURLLoaderHeaderClient>
      url_loader_header_client_receiver_{this};
  const content::ContentBrowserClient::URLLoaderFactoryType
      loader_factory_type_;

  // Mapping from our own internally generated request ID to an
  // InProgressRequest instance.
  std::map<uint64_t, std::unique_ptr<InProgressRequest>> requests_;

  // A mapping from the network stack's notion of request ID to our own
  // internally generated request ID for the same request.
  std::map<int32_t, uint64_t> network_request_id_to_web_request_id_;
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_BROWSER_NET_APP_RUNTIME_PROXYING_URL_LOADER_FACTORY_H_
