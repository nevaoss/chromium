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

#include "neva/pal_service/luna/luna_client_impl.h"

#include "base/logging.h"

#include <glib.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory>

namespace pal {
namespace luna {
namespace {

struct Error : LSError {
  Error() {
    LSErrorInit(this);
  }

  ~Error() {
    LSErrorFree(this);
  }
};

void LogError(Error& error) {
    LOG(ERROR) << error.error_code << ": "
               << error.message << " ("
               << error.func << " @ "
               << error.file << ":"
               << error.line << ")";
}

}  // namespace

ClientImpl::ClientImpl(const Params& params)
  : params_(params) {
  Error error;
  bool registered = false;

  // (stanislav.pikulik@lge.com):
  // Connection to the Luna-bus is possible with the permissions defined for the
  // executable module (WebAppMgr, app_shell, browser_shell) or with the
  // permissions of the nested application that runs in the container.
  // WebAppMgr, app_shell and browser_shell are defined as containers. Nested
  // applications are designated via appid in webOS and they are WebApplications
  // in case of running in WebAppMgr container.
  //
  // LunaClient is a fairly low-level wrapper over lunaservice2 client API with
  // a simple logic of operation:
  // * If the params.appid is empty, it means that the permissions for
  //   executable module will be applied. They are defined by
  //   webos_device:/usr/share/luna-service2/roles.d/<name>.role.json
  //   where "exeName":"/usr/bin/<exe_module_name>" field is provided.
  //
  // * If the params.appid is not empty, it means that the permissions
  //   for nested application will be applied. These permissions are defined
  //   by webos_device:/usr/share/luna-service2/roles.d/<name>.role.json
  //   where "appId": "<app_id>" field is provided.
  //
  // The simple conclusion is that if you need to connect to the Luna-bus on
  // behalf of the executable module, it is enough to leave the params.appid
  // empty. Changing the connection method for one specific appid
  // (luna::service_name::kSettingsClient) breaks the logic of LunaClient.
  //
  // There are two ways.
  //
  // 1) The most correct way is to simply define the right permissions for the
  //    kSettingsClient. Since the change I'm warning against are related to
  //    the WAM and ErrorPage, the permission files should be provided by the
  //    wam bitbake target.
  //    In the Neva project we have defined these files in the
  //    webruntime:src/webos/install/error_page_permission directory.
  //    It's better to choose better name for them and move to wam target.
  //
  // 2) params.appid can be clean specifically for the kSettingsClient in the
  //    SystemServiceBridgeDelegateWebOS. However, this breaks the logic of the
  //    SystemServiceBridgeDelegateWebOS and webosservicebridge JS API, as it
  //    is designed to work on behalf of the web-application. Nevertheless, it
  //    is a little better then in LunaClientImpl. The LunaClientImpl is used
  //    not only by the SystemServiceBridgeDelegateWebOS. This approach limits
  //    the impact of the workaround to the limits of the
  //    SystemServiceBridgeDelegateWebOS.
  //
  //    SystemServiceBridgeDelegateWebOS::SystemServiceBridgeDelegateWebOS(
  //        CreationParams params,
  //        Response callback)
  //        : callback_(std::move(callback)), weak_factory_(this) {
  //      ...
  //      luna_params.appid = std::move(params.appid);
  //      if (luna_params.appid.find(luna::service_name::kSettingsClient) == 0)
  //        luna_params.appid.clear();
  //      luna_client_ = luna::GetSharedClient(luna_params);
  //    }

  // Please read the comment above before adding kSettingClient to the condition
  if (params.appid.empty()) {
    registered = LSRegister(params.name.c_str(), &handle_, &error);
  } else {
    registered = LSRegisterApplicationService(
        params.name.c_str(), params.appid.c_str(), &handle_, &error);
  }

  if (!registered) {
    LogError(error);
    return;
  }

  context_ = g_main_context_ref(g_main_context_default());
  if (!LSGmainContextAttach(handle_, context_, &error)) {
    LogError(error);
    if (!LSUnregister(handle_, &error))
      LogError(error);
    g_main_context_unref(context_);
    context_ = nullptr;
    handle_ = nullptr;
  }
}

ClientImpl::~ClientImpl() {
  CancelAllSubscriptions();
  CancelWaitingCalls();

  Error error;
  if (handle_) {
    if (!LSUnregister(handle_, &error))
      LogError(error);
  }
  if (context_)
    g_main_context_unref(context_);
}

bool ClientImpl::IsInitialized() const {
  return handle_ != nullptr;
}

std::string ClientImpl::GetName() const {
  return params_.name;
}

std::string ClientImpl::GetAppId() const {
  return params_.appid;
}

bool ClientImpl::Call(std::string uri,
                      std::string param,
                      OnceResponse callback,
                      std::string on_cancel_value,
                      unsigned* token) {
  return CallFromApp(std::move(uri),
                     std::move(param),
                     std::string(),
                     std::move(callback),
                     std::move(on_cancel_value),
                     token);
}

bool ClientImpl::CallFromApp(std::string uri,
                             std::string param,
                             std::string app_id,
                             OnceResponse callback,
                             std::string on_cancel_value,
                             unsigned* token) {
  if (!handle_)
    return false;

  Error error;
  auto response = std::make_unique<Response>();
  response->callback = std::move(callback);
  response->context.ptr = this;
  response->context.uri = std::move(uri);
  response->context.param = std::move(param);
  response->context.app_id = std::move(app_id);
  response->context.on_cancel_value = std::move(on_cancel_value);

  const char* app_id_str = response->context.app_id.empty()
                         ? nullptr : response->context.app_id.c_str();

  if (!LSCallFromApplicationOneReply(handle_,
                                     response->context.uri.c_str(),
                                     response->context.param.c_str(),
                                     app_id_str,
                                     HandleResponse,
                                     response.get(),
                                     &(response->context.token),
                                     &error)) {
    LogError(error);
    std::move(response->callback).Run(ResponseStatus::ERROR,
                            static_cast<unsigned>(response->context.token),
                            response->context.on_cancel_value);
    return false;
  }

  unsigned ret_token = static_cast<unsigned>(response->context.token);
  responses_[ret_token] = std::move(response);

  if (token)
    *token = ret_token;
  return true;
}

bool ClientImpl::Subscribe(std::string uri,
                           std::string param,
                           RepeatingResponse callback,
                           std::string on_cancel_value,
                           unsigned* token) {
  return SubscribeFromApp(std::move(uri),
                          std::move(param),
                          std::string(),
                          std::move(callback),
                          std::move(on_cancel_value),
                          token);
}

bool ClientImpl::SubscribeFromApp(std::string uri,
                                  std::string param,
                                  std::string app_id,
                                  RepeatingResponse callback,
                                  std::string on_cancel_value,
                                  unsigned* token) {
  if (!handle_)
    return false;

  Error error;
  auto subscription = std::make_unique<Subscription>();
  subscription->callback = std::move(callback);
  subscription->context.ptr = this;
  subscription->context.uri = std::move(uri);
  subscription->context.param = std::move(param);
  subscription->context.app_id = std::move(app_id);
  subscription->context.on_cancel_value = std::move(on_cancel_value);

  const char* app_id_str = subscription->context.app_id.empty()
                         ? nullptr : subscription->context.app_id.c_str();

  if (!LSCallFromApplication(handle_,
              subscription->context.uri.c_str(),
              subscription->context.param.c_str(),
              app_id_str,
              HandleSubscribe,
              subscription.get(),
              &(subscription->context.token),
              &error)) {
    LogError(error);
    subscription->callback.Run(ResponseStatus::ERROR,
                 static_cast<unsigned>(subscription->context.token),
                 subscription->context.on_cancel_value);
    return false;
  }

  unsigned ret_token = static_cast<unsigned>(subscription->context.token);
  subscriptions_[ret_token] = std::move(subscription);

  if (token)
    *token = ret_token;
  return true;
}

void ClientImpl::Cancel(unsigned token) {
  if (!handle_)
    return;

  auto it = responses_.find(token);
  if (it == responses_.end())
    return;

  LSMessageToken key = it->second->context.token;
  Error error;
  if (!LSCallCancel(handle_, key, &error))
    LOG(INFO) << "[CANCEL] " << key << " fail [" << error.message << "]";

  std::move(it->second->callback)
      .Run(ResponseStatus::CANCELED, token,
           it->second->context.on_cancel_value);
  responses_.erase(it);
}

void ClientImpl::Unsubscribe(unsigned token) {
  if (!handle_)
    return;

  auto it = subscriptions_.find(token);
  if (it == subscriptions_.end())
    return;

  Error error;
  LSMessageToken key = it->second->context.token;
  if (!LSCallCancel(handle_, key, &error))
    LOG(INFO) << "[UNSUB] " << key << " fail [" << error.message << "]";
  std::move(it->second->callback)
      .Run(ResponseStatus::CANCELED, token,
           it->second->context.on_cancel_value);
  subscriptions_.erase(it);
}

void ClientImpl::CancelAllSubscriptions() {
  if (!handle_)
    return;

  Error error;
  for (auto& subscription : subscriptions_) {
    LSMessageToken key = subscription.second->context.token;
    if (!LSCallCancel(handle_, key, &error))
      LOG(INFO) << "[UNSUB] " << key << " fail [" << error.message << "]";

    std::move(subscription.second->callback)
        .Run(ResponseStatus::CANCELED,
             subscription.second->context.token,
             subscription.second->context.on_cancel_value);
  }
  subscriptions_.clear();
}

void ClientImpl::CancelWaitingCalls() {
  if (!handle_)
    return;

  Error error;
  for (auto& response : responses_) {
    LSMessageToken key = response.second->context.token;
    if (!LSCallCancel(handle_, key, &error))
      LOG(INFO) << "[CANCEL] " << key << " fail [" << error.message << "]";
    std::move(response.second->callback)
        .Run(ResponseStatus::CANCELED,
             response.second->context.token,
             response.second->context.on_cancel_value);
  }
  responses_.clear();
}

// static
bool ClientImpl::HandleResponse(LSHandle* sh, LSMessage* reply, void* ctx) {
  ClientImpl::Response* response = static_cast<ClientImpl::Response*>(ctx);
  if (response && response->context.ptr) {
    auto self = response->context.ptr;
    auto it = self->responses_.find(response->context.token);
    if (it != self->responses_.end()) {
      LSMessageRef(reply);
      const std::string dump = LSMessageGetPayload(reply);
      std::move(response->callback).Run(
          ResponseStatus::SUCCESS, response->context.token, dump);
      self->responses_.erase(it);
      LSMessageUnref(reply);
    }
  } else {
    NOTREACHED();
    delete response;
  }

  return true;
}

// static
bool ClientImpl::HandleSubscribe(LSHandle* sh, LSMessage* reply, void* ctx) {
  ClientImpl::Subscription* subscription =
    static_cast<ClientImpl::Subscription*>(ctx);
  if (subscription) {
    LSMessageRef(reply);
    const std::string dump = LSMessageGetPayload(reply);
    subscription->callback.Run(
        ResponseStatus::SUCCESS, subscription->context.token, dump);
    LSMessageUnref(reply);
  } else {
    NOTREACHED();
    delete subscription;
  }

  return true;
}

}  // namespace luna
}  // namespace pal
