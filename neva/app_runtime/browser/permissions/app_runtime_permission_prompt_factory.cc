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

#include "neva/app_runtime/browser/permissions/app_runtime_permission_prompt_factory.h"

#include "neva/app_runtime/browser/permissions/app_runtime_permission_prompt.h"

namespace neva_app_runtime {

AppRuntimePermissionPromptFactory::~AppRuntimePermissionPromptFactory() {}

std::unique_ptr<permissions::PermissionPrompt>
AppRuntimePermissionPromptFactory::CreatePermissionPrompt(
    content::WebContents* web_contents,
    permissions::PermissionPrompt::Delegate* delegate) {
  auto ptr =
      std::make_unique<AppRuntimePermissionPrompt>(web_contents, delegate);
  ptr->ShowBubble();
  return ptr;
}

}  // namespace neva_app_runtime
