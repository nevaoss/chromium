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

// Based on
// //chrome/browser/extensions/api/extension_action/extension_action_api.cc

// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "neva/extensions/browser/api/extension_action/extension_action_api.h"

#include "base/lazy_instance.h"
#include "base/values.h"
#include "content/public/browser/web_contents.h"
#include "extensions/browser/event_router.h"
#include "extensions/browser/extension_action_manager.h"
#include "extensions/common/manifest_constants.h"
#include "neva/extensions/browser/neva_extensions_service_factory.h"
#include "neva/extensions/browser/neva_extensions_service_impl.h"
#include "neva/extensions/browser/tab_helper.h"

namespace neva {

const int kDefaultTabId = 0;

// Errors.
const char kNoExtensionActionError[] =
    "This extension has no action specified.";
const char kNoTabError[] = "No tab with id: *.";

ExtensionActionAPI::ExtensionActionAPI(content::BrowserContext* context)
    : browser_context_(context) {}

ExtensionActionAPI::~ExtensionActionAPI() {}

static base::LazyInstance<extensions::BrowserContextKeyedAPIFactory<
    ExtensionActionAPI>>::DestructorAtExit g_extension_action_api_factory =
    LAZY_INSTANCE_INITIALIZER;

// static
extensions::BrowserContextKeyedAPIFactory<ExtensionActionAPI>*
ExtensionActionAPI::GetFactoryInstance() {
  return g_extension_action_api_factory.Pointer();
}

// static
ExtensionActionAPI* ExtensionActionAPI::Get(content::BrowserContext* context) {
  return extensions::BrowserContextKeyedAPIFactory<ExtensionActionAPI>::Get(
      context);
}

void ExtensionActionAPI::DispatchExtensionActionClicked(
    const std::string& extension_id,
    const uint64_t tab_id,
    content::WebContents* web_contents) {
  // TODO(neva): In chrome browser, there are much more arguments such as
  // favIconUrl, width/height, index, openerTabId, title, windowId.
  base::Value::List args;
  base::Value::Dict dict;
  dict.Set("url", web_contents->GetLastCommittedURL().spec());
  dict.Set("id", static_cast<int>(tab_id));
  args.Append(std::move(dict));

  extensions::events::HistogramValue histogram_value =
      extensions::events::UNKNOWN;
  auto event = std::make_unique<extensions::Event>(
      histogram_value, "action.onClicked", std::move(args), browser_context_);
  event->user_gesture = extensions::EventRouter::USER_GESTURE_ENABLED;
  extensions::EventRouter::Get(browser_context_)
      ->DispatchEventToExtension(extension_id, std::move(event));
}

void ExtensionActionAPI::Shutdown() {}

//
// ExtensionActionFunction
//

ExtensionActionFunction::ExtensionActionFunction()
    : details_(nullptr),
      tab_id_(kDefaultTabId),
      contents_(nullptr),
      extension_action_(nullptr) {}

ExtensionActionFunction::~ExtensionActionFunction() {}

ExtensionFunction::ResponseAction ExtensionActionFunction::Run() {
  extensions::ExtensionActionManager* manager =
      extensions::ExtensionActionManager::Get(browser_context());
  extension_action_ = manager->GetExtensionAction(*extension());
  if (!extension_action_) {
    // TODO(kalman): ideally the browserAction/pageAction APIs wouldn't event
    // exist for extensions that don't have one declared. This should come as
    // part of the Feature system.
    return RespondNow(Error(kNoExtensionActionError));
  }

  // Populates the tab_id_ and details_ members.
  EXTENSION_FUNCTION_VALIDATE(ExtractDataFromArguments());

  // Find the WebContents that contains this tab id if one is required.
  if (tab_id_ != kDefaultTabId) {
    contents_ = NevaExtensionsServiceFactory::GetService(browser_context())
                    ->GetTabHelper()
                    ->GetWebContentsFromId(tab_id_);
    if (!contents_) {
      return RespondNow(Error(kNoTabError, base::NumberToString(tab_id_)));
    }
  } else {
    // Page actions do not have a default tabId.
    EXTENSION_FUNCTION_VALIDATE(extension_action_->action_type() !=
                                extensions::ActionInfo::Type::kPage);
  }
  return RunExtensionAction();
}

bool ExtensionActionFunction::ExtractDataFromArguments() {
  // There may or may not be details (depends on the function).
  // The tabId might appear in details (if it exists), as the first
  // argument besides the action type (depends on the function), or be omitted
  // entirely.
  if (args().empty()) {
    return true;
  }

  base::Value& first_arg = mutable_args()[0];

  switch (first_arg.type()) {
    case base::Value::Type::INTEGER:
      tab_id_ = first_arg.GetInt();
      break;

    case base::Value::Type::DICT: {
      // Found the details argument.
      details_ = &first_arg.GetDict();
      // Still need to check for the tabId within details.
      if (base::Value* tab_id_value = details_->Find("tabId")) {
        switch (tab_id_value->type()) {
          case base::Value::Type::NONE:
            // OK; tabId is optional, leave it default.
            return true;
          case base::Value::Type::INTEGER:
            tab_id_ = tab_id_value->GetInt();
            return true;
          default:
            // Boom.
            return false;
        }
      }
      // Not found; tabId is optional, leave it default.
      break;
    }

    case base::Value::Type::NONE:
      // The tabId might be an optional argument.
      break;

    default:
      return false;
  }

  return true;
}

void ExtensionActionFunction::SetVisible(bool visible) {
  if (extension_action_->GetIsVisible(tab_id_) == visible) {
    return;
  }
  extension_action_->SetIsVisible(tab_id_, visible);
}

ExtensionFunction::ResponseAction
ExtensionActionShowFunction::RunExtensionAction() {
  SetVisible(true);
  return RespondNow(NoArguments());
}

ExtensionFunction::ResponseAction
ExtensionActionSetIconFunction::RunExtensionAction() {
  return RespondNow(NoArguments());
}

ExtensionFunction::ResponseAction
ExtensionActionSetPopupFunction::RunExtensionAction() {
  EXTENSION_FUNCTION_VALIDATE(details_);
  std::string* popup_string = details_->FindString("popup");
  EXTENSION_FUNCTION_VALIDATE(popup_string);
  GURL popup_url;

  // If an empty string is passed, remove the explicitly set popup. Setting it
  // back to an empty string (URL) will cause it to fall back to the default set
  // in the manifest.
  if (!popup_string->empty()) {
    popup_url = extension()->GetResourceURL(*popup_string);
    // Validate popup is same-origin (only for this extension). We do not
    // validate the file exists (like we do in manifest validation) because an
    // extension could potentially intercept the request with a service worker
    // and dynamically provide content.
    if (!extension()->origin().IsSameOriginWith(popup_url)) {
      return RespondNow(
          Error(extensions::manifest_errors::kInvalidExtensionOriginPopup));
    }
  }

  extension_action_->SetPopupUrl(tab_id_, popup_url);

  return RespondNow(NoArguments());
}

ExtensionFunction::ResponseAction
ExtensionActionSetBadgeTextFunction::RunExtensionAction() {
  return RespondNow(NoArguments());
}

ExtensionFunction::ResponseAction
ExtensionActionSetBadgeBackgroundColorFunction::RunExtensionAction() {
  return RespondNow(NoArguments());
}

ExtensionFunction::ResponseAction
ExtensionActionGetPopupFunction::RunExtensionAction() {
  return RespondNow(
      WithArguments(extension_action_->GetPopupUrl(tab_id_).spec()));
}

}  // namespace neva
