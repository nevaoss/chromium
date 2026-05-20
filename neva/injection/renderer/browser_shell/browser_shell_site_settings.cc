// Copyright 2025 LG Electronics, Inc.
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

#include "neva/injection/renderer/browser_shell/browser_shell_site_settings.h"

#include "base/functional/bind.h"
#include "base/logging.h"
#include "gin/arguments.h"
#include "gin/dictionary.h"

namespace injections {

BrowserShellSiteSettings::BrowserShellSiteSettings(
    v8::Isolate* isolate,
    mojo::Remote<browser_shell::mojom::SiteSettings> remote)
    : remote_(std::move(remote)) {
  remote_->RegisterClient(client_receiver_.BindNewEndpointAndPassRemote());
}

BrowserShellSiteSettings::~BrowserShellSiteSettings() = default;

void BrowserShellSiteSettings::GetAllSites(gin::Arguments* args) {
  v8::Isolate* isolate = args->isolate();
  v8::Local<v8::Function> callback;
  if (args->GetNext(&callback)) {
    remote_->GetAllSites(
        base::BindOnce(&BrowserShellSiteSettings::OnGetSitesReply, GetAsWeak(),
                       v8::Global<v8::Function>(isolate, callback)));
  }
}

void BrowserShellSiteSettings::GetSitesForSettingType(gin::Arguments* args) {
  v8::Isolate* isolate = args->isolate();

  std::string type;
  if (!args->GetNext(&type)) {
    return;
  }

  v8::Local<v8::Function> callback;
  if (args->GetNext(&callback)) {
    remote_->GetSitesForSettingType(
        type,
        base::BindOnce(&BrowserShellSiteSettings::OnGetSitesReply, GetAsWeak(),
                       v8::Global<v8::Function>(isolate, callback)));
  }
}

void BrowserShellSiteSettings::GetOriginPermissions(gin::Arguments* args) {
  v8::Isolate* isolate = args->isolate();

  std::string origin;
  if (!args->GetNext(&origin)) {
    return;
  }

  v8::Local<v8::Value> arg2_val;
  if (!args->GetNext(&arg2_val)) {
    return;
  }

  v8::Local<v8::Function> callback;
  std::vector<std::string> types;
  if (gin::ConvertFromV8(isolate, arg2_val, &types)) {
    if (!args->GetNext(&callback)) {
      return;
    }
  } else if (!gin::ConvertFromV8(isolate, arg2_val, &callback)) {
    return;
  }

  remote_->GetOriginPermissions(
      origin, types,
      base::BindOnce(&BrowserShellSiteSettings::OnGetOriginPermissionsReply,
                     GetAsWeak(), v8::Global<v8::Function>(isolate, callback)));
}

void BrowserShellSiteSettings::ResetOriginPermissions(gin::Arguments* args) {
  std::string origin;
  if (!args->GetNext(&origin)) {
    return;
  }

  std::vector<std::string> types;
  if (!args->GetNext(&types) && args->Length() > 1) {
    return;
  }

  remote_->ResetOriginPermissions(origin, types);
}

void BrowserShellSiteSettings::OnUpdate() {}

void BrowserShellSiteSettings::Trace(cppgc::Visitor* visitor) const {
  visitor->Trace(weak_factory_);
  gin::Wrappable<BrowserShellSiteSettings>::Trace(visitor);
}

void BrowserShellSiteSettings::OnGetSitesReply(
    v8::Global<v8::Function> callback,
    const std::vector<std::string>& sites) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Object> wrapper;
  if (!GetWrapper(isolate).ToLocal(&wrapper)) {
    LOG(ERROR) << __func__ << "(): cannot get wrapper";
    return;
  }

  v8::Local<v8::Context> context;
  if (!wrapper->GetCreationContext().ToLocal(&context)) {
    LOG(ERROR) << __func__ << "(): cannot get context";
    return;
  }

  v8::MicrotasksScope microtasksScope(context,
                                      v8::MicrotasksScope::kRunMicrotasks);
  v8::Context::Scope context_scope(context);

  const int argc = 1;
  v8::Local<v8::Value> argv[] = {gin::ConvertToV8(isolate, sites)};
  std::ignore = callback.Get(isolate)->Call(context, wrapper, argc, argv);
}

void BrowserShellSiteSettings::OnGetOriginPermissionsReply(
    v8::Global<v8::Function> callback,
    std::vector<browser_shell::mojom::PermissionPtr> permissions) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Object> wrapper;
  if (!GetWrapper(isolate).ToLocal(&wrapper)) {
    LOG(ERROR) << __func__ << "(): cannot get wrapper";
    return;
  }

  v8::Local<v8::Context> context;
  if (!wrapper->GetCreationContext().ToLocal(&context)) {
    LOG(ERROR) << __func__ << "(): cannot get context";
    return;
  }

  v8::MicrotasksScope microtasksScope(context,
                                      v8::MicrotasksScope::kRunMicrotasks);
  v8::Context::Scope context_scope(context);
  v8::Local<v8::Array> permissions_arr =
      v8::Array::New(isolate, permissions.size());

  for (size_t i = 0; i < permissions.size(); ++i) {
    v8::Local<v8::Object> perm_object = v8::Object::New(isolate);
    gin::Dictionary perm_dict(isolate, perm_object);
    perm_dict.Set("type", permissions[i]->type);
    perm_dict.Set("setting", permissions[i]->setting);
    permissions_arr->Set(context, i, perm_object).Check();
  }

  const int argc = 1;
  v8::Local<v8::Value> argv[] = {permissions_arr};
  std::ignore = callback.Get(isolate)->Call(context, wrapper, argc, argv);
}

gin::ObjectTemplateBuilder BrowserShellSiteSettings::GetObjectTemplateBuilder(
    v8::Isolate* isolate) {
  return gin::Wrappable<BrowserShellSiteSettings>::GetObjectTemplateBuilder(
             isolate)
      .SetMethod("getAllSites", &BrowserShellSiteSettings::GetAllSites)
      .SetMethod("getSitesForSettingType",
                 &BrowserShellSiteSettings::GetSitesForSettingType)
      .SetMethod("getOriginPermissions",
                 &BrowserShellSiteSettings::GetOriginPermissions)
      .SetMethod("resetOriginPermissions",
                 &BrowserShellSiteSettings::ResetOriginPermissions);
}

const gin::WrapperInfo* BrowserShellSiteSettings::wrapper_info() const {
  return &kWrapperInfo;
}

}  // namespace injections
