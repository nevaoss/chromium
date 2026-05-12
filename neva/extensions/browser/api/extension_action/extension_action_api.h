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
// //chrome/browser/extensions/api/extension_action/extension_action_api.h

// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NEVA_EXTENSIONS_BROWSER_API_EXTENSION_ACTION_EXTENSION_ACTION_API_H_
#define NEVA_EXTENSIONS_BROWSER_API_EXTENSION_ACTION_EXTENSION_ACTION_API_H_

#include "extensions/browser/browser_context_keyed_api_factory.h"
#include "extensions/browser/extension_action.h"
#include "extensions/browser/extension_function.h"

namespace content {
class BrowserContext;
}

namespace neva {

class ExtensionActionAPI : public extensions::BrowserContextKeyedAPI {
 public:
  explicit ExtensionActionAPI(content::BrowserContext* context);

  ExtensionActionAPI(const ExtensionActionAPI&) = delete;
  ExtensionActionAPI& operator=(const ExtensionActionAPI&) = delete;

  ~ExtensionActionAPI() override;

  static ExtensionActionAPI* Get(content::BrowserContext* context);

  static extensions::BrowserContextKeyedAPIFactory<ExtensionActionAPI>*
  GetFactoryInstance();

  // Dispatches the onClicked event for extension that owns the given action.
  void DispatchExtensionActionClicked(const std::string& extension_id,
                                      const uint64_t tab_id,
                                      content::WebContents* web_contents);

 private:
  friend class extensions::BrowserContextKeyedAPIFactory<ExtensionActionAPI>;

  // BrowserContextKeyedAPI implementation.
  void Shutdown() override;
  static const char* service_name() { return "ExtensionActionAPI"; }

  raw_ptr<content::BrowserContext> browser_context_;
};

class ExtensionActionFunction : public ExtensionFunction {
 protected:
  ExtensionActionFunction();
  ~ExtensionActionFunction() override;
  ResponseAction Run() override;

  virtual ResponseAction RunExtensionAction() = 0;

  bool ExtractDataFromArguments();
  void SetVisible(bool visible);

  // All the extension action APIs take a single argument called details that
  // is a dictionary.
  raw_ptr<base::Value::Dict> details_;

  // The tab id the extension action function should apply to, if any, or
  // kDefaultTabId if none was specified.
  int tab_id_;

  // WebContents for |tab_id_| if one exists.
  // This field is not a raw_ptr<> because it was filtered by the rewriter for:
  // #addr-of
  RAW_PTR_EXCLUSION content::WebContents* contents_;

  // The extension action for the current extension.
  raw_ptr<extensions::ExtensionAction> extension_action_;
};

//
// Implementations of each extension action API.
//
// pageAction and browserAction bindings are created for these by extending them
// then declaring an EXTENSION_FUNCTION_NAME.
//

// show
class ExtensionActionShowFunction : public ExtensionActionFunction {
 protected:
  ~ExtensionActionShowFunction() override {}
  ResponseAction RunExtensionAction() override;
};

// setIcon
class ExtensionActionSetIconFunction : public ExtensionActionFunction {
 protected:
  ~ExtensionActionSetIconFunction() override {}
  ResponseAction RunExtensionAction() override;
};

class ActionSetIconFunction : public ExtensionActionSetIconFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("action.setIcon", ACTION_SETICON)

 protected:
  ~ActionSetIconFunction() override {}
};

class BrowserActionSetIconFunction : public ExtensionActionSetIconFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("browserAction.setIcon", BROWSERACTION_SETICON)

 protected:
  ~BrowserActionSetIconFunction() override {}
};

// setPopup
class ExtensionActionSetPopupFunction : public ExtensionActionFunction {
 protected:
  ~ExtensionActionSetPopupFunction() override {}
  ResponseAction RunExtensionAction() override;
};

class ActionSetPopupFunction : public ExtensionActionSetPopupFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("action.setPopup", ACTION_SETPOPUP)

 protected:
  ~ActionSetPopupFunction() override {}
};

// setBadgeText
class ExtensionActionSetBadgeTextFunction : public ExtensionActionFunction {
 protected:
  ~ExtensionActionSetBadgeTextFunction() override {}
  ResponseAction RunExtensionAction() override;
};

// setBadgeBackgroundColor
class ExtensionActionSetBadgeBackgroundColorFunction
    : public ExtensionActionFunction {
 protected:
  ~ExtensionActionSetBadgeBackgroundColorFunction() override {}
  ResponseAction RunExtensionAction() override;
};

// getPopup
class ExtensionActionGetPopupFunction : public ExtensionActionFunction {
 protected:
  ~ExtensionActionGetPopupFunction() override {}
  ResponseAction RunExtensionAction() override;
};

class BrowserActionGetPopupFunction : public ExtensionActionGetPopupFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("browserAction.getPopup", BROWSERACTION_GETPOPUP)

 protected:
  ~BrowserActionGetPopupFunction() override {}
};

class ActionSetBadgeTextFunction : public ExtensionActionSetBadgeTextFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("action.setBadgeText", ACTION_SETBADGETEXT)

 protected:
  ~ActionSetBadgeTextFunction() override {}
};

class ActionSetBadgeBackgroundColorFunction
    : public ExtensionActionSetBadgeBackgroundColorFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("action.setBadgeBackgroundColor",
                             ACTION_SETBADGEBACKGROUNDCOLOR)

 protected:
  ~ActionSetBadgeBackgroundColorFunction() override {}
};

class BrowserActionSetBadgeTextFunction
    : public ExtensionActionSetBadgeTextFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("browserAction.setBadgeText",
                             BROWSERACTION_SETBADGETEXT)

 protected:
  ~BrowserActionSetBadgeTextFunction() override {}
};

class BrowserActionSetBadgeBackgroundColorFunction
    : public ExtensionActionSetBadgeBackgroundColorFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("browserAction.setBadgeBackgroundColor",
                             BROWSERACTION_SETBADGEBACKGROUNDCOLOR)

 protected:
  ~BrowserActionSetBadgeBackgroundColorFunction() override {}
};

class PageActionShowFunction : public ExtensionActionShowFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("pageAction.show", PAGEACTION_SHOW)

 protected:
  ~PageActionShowFunction() override {}
};

class PageActionSetPopupFunction : public ExtensionActionSetPopupFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("pageAction.setPopup", PAGEACTION_SETPOPUP)

 protected:
  ~PageActionSetPopupFunction() override {}
};

}  // namespace neva

#endif  // NEVA_EXTENSIONS_BROWSER_API_EXTENSION_ACTION_EXTENSION_ACTION_API_H_
