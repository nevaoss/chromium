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

#include "neva/app_runtime/app/app_runtime_js_dialog_manager.h"

#include "base/logging.h"
#include "base/notreached.h"
#include "base/strings/utf_string_conversions.h"
#include "neva/app_runtime/app/app_runtime_js_dialog_manager_delegate.h"
#include "neva/logging.h"

namespace neva_app_runtime {

JSDialogManager::JSDialogManager(JSDialogManagerDelegate* delegate)
    : delegate_(delegate) {}

JSDialogManager::~JSDialogManager() = default;

void JSDialogManager::RunJavaScriptDialog(
    content::WebContents* web_contents,
    content::RenderFrameHost* render_frame_host,
    content::JavaScriptDialogType dialog_type,
    const std::u16string& message_text,
    const std::u16string& default_prompt_text,
    DialogClosedCallback callback,
    bool* did_suppress_message) {
  if (!callback_.is_null()) {
    // JavaScript Dialogs are modal.
    LOG(WARNING) << "Previous dialog has not been closed.";
    std::move(callback_).Run(false, u"");
  }

  callback_ = std::move(callback);

  std::string type;
  if (dialog_type == content::JAVASCRIPT_DIALOG_TYPE_ALERT)
    type = "alert";
  else if (dialog_type == content::JAVASCRIPT_DIALOG_TYPE_CONFIRM)
    type = "confirm";
  else if (dialog_type == content::JAVASCRIPT_DIALOG_TYPE_PROMPT)
    type = "prompt";
  else
    NOTREACHED() << "Unknown JavaScript Dialog Type.";

  std::string msg = base::UTF16ToUTF8(message_text);
  *did_suppress_message = !delegate_->RunJSDialog(type, msg);
  if (*did_suppress_message)
    callback_.Reset();
}

void JSDialogManager::RunBeforeUnloadDialog(
    content::WebContents* web_contents,
    content::RenderFrameHost* render_frame_host,
    bool is_reload,
    DialogClosedCallback callback) {
  NOTIMPLEMENTED();
  std::move(callback).Run(false, u"");
}

bool JSDialogManager::HandleJavaScriptDialog(
    content::WebContents* web_contents,
    bool accept,
    const std::u16string* prompt_override) {
  NOTIMPLEMENTED();
  return false;
}

void JSDialogManager::CancelDialogs(content::WebContents* web_contents,
                                    bool reset_state) {
  NOTIMPLEMENTED();
}

void JSDialogManager::CloseDialog(bool success, const std::string& response) {
  if (callback_.is_null()) {
    LOG(WARNING) << __func__ << ", There is no active JavaScript dialog.";
    return;
  }
  std::move(callback_).Run(success, base::UTF8ToUTF16(response));
}

}  // namespace neva_app_runtime
