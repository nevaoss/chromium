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

#ifndef NEVA_APP_RUNTIME_BROWSER_PERMISSIONS_APP_RUNTIME_PERMISSION_PROMPT_H_
#define NEVA_APP_RUNTIME_BROWSER_PERMISSIONS_APP_RUNTIME_PERMISSION_PROMPT_H_

#include "base/memory/raw_ptr.h"
#include "components/permissions/permission_prompt.h"

namespace content {
class WebContents;
}  // namespace content

namespace neva_app_runtime {

// This object will create or trigger system UI to reflect that a website is
// requesting a permission.
class AppRuntimePermissionPrompt : public permissions::PermissionPrompt {
 public:
  AppRuntimePermissionPrompt(content::WebContents* web_contents,
                             permissions::PermissionPrompt::Delegate* delegate);
  AppRuntimePermissionPrompt(const AppRuntimePermissionPrompt&) = delete;
  AppRuntimePermissionPrompt& operator=(const AppRuntimePermissionPrompt&) =
      delete;

  ~AppRuntimePermissionPrompt() override;

  // permissions::PermissionPrompt:
  bool UpdateAnchor() override;
  TabSwitchingBehavior GetTabSwitchingBehavior() override;

  permissions::PermissionPromptDisposition GetPromptDisposition()
      const override;

  bool IsAskPrompt() const override;
  std::optional<gfx::Rect> GetViewBoundsInScreen() const override;
  bool ShouldFinalizeRequestAfterDecided() const override;

  std::vector<permissions::ElementAnchoredBubbleVariant> GetPromptVariants()
      const override;

  std::optional<permissions::feature_params::PermissionElementPromptPosition>
  GetPromptPosition() const override;

  // Show a small pop-up window with the prompt.
  virtual void ShowBubble();

 protected:
  base::WeakPtr<permissions::PermissionPrompt::Delegate> delegate_;
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_BROWSER_PERMISSIONS_APP_RUNTIME_PERMISSION_PROMPT_H_
