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

#ifndef NEVA_APP_RUNTIME_APP_APP_RUNTIME_JS_DIALOG_MANAGER_H_
#define NEVA_APP_RUNTIME_APP_APP_RUNTIME_JS_DIALOG_MANAGER_H_

#include <string>

#include "base/functional/callback.h"
#include "base/memory/raw_ptr.h"
#include "content/public/browser/javascript_dialog_manager.h"

namespace neva_app_runtime {

class JSDialogManagerDelegate;

class JSDialogManager : public content::JavaScriptDialogManager {
 public:
  explicit JSDialogManager(JSDialogManagerDelegate* delegate);
  JSDialogManager(const JSDialogManager&) = delete;
  JSDialogManager& operator=(const JSDialogManager&) = delete;
  ~JSDialogManager() override;

  void RunJavaScriptDialog(content::WebContents* web_contents,
                           content::RenderFrameHost* render_frame_host,
                           content::JavaScriptDialogType dialog_type,
                           const std::u16string& message_text,
                           const std::u16string& default_prompt_text,
                           DialogClosedCallback callback,
                           bool* did_suppress_message) override;

  void RunBeforeUnloadDialog(content::WebContents* web_contents,
                             content::RenderFrameHost* render_frame_host,
                             bool is_reload,
                             DialogClosedCallback callback) override;

  bool HandleJavaScriptDialog(content::WebContents* web_contents,
                              bool accept,
                              const std::u16string* prompt_override) override;

  void CancelDialogs(content::WebContents* web_contents,
                     bool reset_state) override;

  void CloseDialog(bool success, const std::string& response);

 private:
  DialogClosedCallback callback_;
  raw_ptr<JSDialogManagerDelegate> delegate_;
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_APP_APP_RUNTIME_JS_DIALOG_MANAGER_H_
