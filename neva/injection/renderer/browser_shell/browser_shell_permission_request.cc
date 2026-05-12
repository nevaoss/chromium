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

#include "neva/injection/renderer/browser_shell/browser_shell_permission_request.h"

namespace injections {

gin::WrapperInfo PermissionRequest::kWrapperInfo = {gin::kEmbedderNativeGin};

PermissionRequest::PermissionRequest(Delegate* delegate,
                                     uint64_t request_id,
                                     uint64_t page_id)
    : delegate_(delegate), request_id_(request_id), page_id_(page_id) {}

PermissionRequest::~PermissionRequest() = default;

gin::ObjectTemplateBuilder PermissionRequest::GetObjectTemplateBuilder(
    v8::Isolate* isolate) {
  return gin::Wrappable<PermissionRequest>::GetObjectTemplateBuilder(isolate)
      .SetMethod("allow", &PermissionRequest::Allow)
      .SetMethod("allowThisTime", &PermissionRequest::AllowThisTime)
      .SetMethod("deny", &PermissionRequest::Deny)
      .SetMethod("dismiss", &PermissionRequest::Dismiss)
      .SetMethod("ignore", &PermissionRequest::Ignore);
}

void PermissionRequest::MakeDecision(Decision result) {
  if (delegate_) {
    delegate_->AckPermission(result, request_id_, page_id_);
    delegate_ = nullptr;
  }
}

void PermissionRequest::Allow() {
  MakeDecision(Decision::kAllow);
}

void PermissionRequest::AllowThisTime() {
  MakeDecision(Decision::kAllowThisTime);
}

void PermissionRequest::Deny() {
  MakeDecision(Decision::kDeny);
}

void PermissionRequest::Dismiss() {
  MakeDecision(Decision::kDismiss);
}

void PermissionRequest::Ignore() {
  MakeDecision(Decision::kIgnore);
}

}  // namespace injections
