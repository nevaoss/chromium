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

#include "neva/app_runtime/browser/permissions/app_runtime_permission_prompt.h"

#include "content/public/browser/web_contents.h"
#include "neva/app_runtime/browser/permissions/app_runtime_permission_prompt.h"
#include "url/gurl.h"

namespace neva_app_runtime {

AppRuntimePermissionPrompt::AppRuntimePermissionPrompt(
    content::WebContents*,
    permissions::PermissionPrompt::Delegate* delegate) {
  if (delegate) {
    delegate_ = delegate->GetWeakPtr();
  }
}

AppRuntimePermissionPrompt::~AppRuntimePermissionPrompt() {}

bool AppRuntimePermissionPrompt::UpdateAnchor() {
  return false;
}

permissions::PermissionPrompt::TabSwitchingBehavior
AppRuntimePermissionPrompt::GetTabSwitchingBehavior() {
  return permissions::PermissionPrompt::TabSwitchingBehavior::
      kDestroyPromptButKeepRequestPending;
}

bool AppRuntimePermissionPrompt::IsAskPrompt() const {
  return true;
}

permissions::PermissionPromptDisposition
AppRuntimePermissionPrompt::GetPromptDisposition() const {
  return permissions::PermissionPromptDisposition::NOT_APPLICABLE;
}

std::optional<gfx::Rect> AppRuntimePermissionPrompt::GetViewBoundsInScreen()
    const {
  return std::nullopt;
}

bool AppRuntimePermissionPrompt::ShouldFinalizeRequestAfterDecided() const {
  return true;
}

std::vector<permissions::ElementAnchoredBubbleVariant>
AppRuntimePermissionPrompt::GetPromptVariants() const {
  return {};
}

std::optional<permissions::feature_params::PermissionElementPromptPosition>
AppRuntimePermissionPrompt::GetPromptPosition() const {
  return std::nullopt;
}

void AppRuntimePermissionPrompt::ShowBubble() {
  if (delegate_) {
    delegate_->Deny();
  }
}

}  // namespace neva_app_runtime
