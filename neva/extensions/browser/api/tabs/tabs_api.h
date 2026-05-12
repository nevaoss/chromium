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

#ifndef NEVA_EXTENSIONS_BROWSER_API_TABS_TABS_API_H_
#define NEVA_EXTENSIONS_BROWSER_API_TABS_TABS_API_H_

#include "base/memory/raw_ptr.h"
#include "components/translate/core/browser/translate_driver.h"
#include "content/public/browser/web_contents_observer.h"
#include "extensions/browser/api/execute_code_function.h"
#include "extensions/browser/api/web_contents_capture_client.h"
#include "extensions/browser/extension_function.h"
#include "extensions/browser/script_executor.h"

namespace neva {

// Windows
class WindowsGetCurrentFunction : public ExtensionFunction {
  ~WindowsGetCurrentFunction() override {}
  ResponseAction Run() override;
  DECLARE_EXTENSION_FUNCTION("windows.getCurrent", WINDOWS_GETCURRENT)
};
class WindowsGetLastFocusedFunction : public ExtensionFunction {
  ~WindowsGetLastFocusedFunction() override {}
  ResponseAction Run() override;
  DECLARE_EXTENSION_FUNCTION("windows.getLastFocused", WINDOWS_GETLASTFOCUSED)
};
class WindowsGetAllFunction : public ExtensionFunction {
  ~WindowsGetAllFunction() override {}
  ResponseAction Run() override;
  DECLARE_EXTENSION_FUNCTION("windows.getAll", WINDOWS_GETALL)
};
class WindowsCreateFunction : public ExtensionFunction {
  ~WindowsCreateFunction() override {}
  ResponseAction Run() override;
  void OnWindowsCreated(int window_id);
  DECLARE_EXTENSION_FUNCTION("windows.create", WINDOWS_CREATE)
};
class WindowsUpdateFunction : public ExtensionFunction {
  ~WindowsUpdateFunction() override {}
  ResponseAction Run() override;
  DECLARE_EXTENSION_FUNCTION("windows.update", WINDOWS_UPDATE)
};
class WindowsRemoveFunction : public ExtensionFunction {
  ~WindowsRemoveFunction() override {}
  ResponseAction Run() override;
  DECLARE_EXTENSION_FUNCTION("windows.remove", WINDOWS_REMOVE)
};

// Tabs
class TabsCreateFunction : public ExtensionFunction {
  ~TabsCreateFunction() override {}
  ResponseAction Run() override;
  void OnTabCreated(int tab_id);
  DECLARE_EXTENSION_FUNCTION("tabs.create", TABS_CREATE)
};
class TabsQueryFunction : public ExtensionFunction {
  ~TabsQueryFunction() override {}
  ResponseAction Run() override;
  DECLARE_EXTENSION_FUNCTION("tabs.query", TABS_QUERY)
};

class TabsRemoveFunction : public ExtensionFunction {
 public:
  TabsRemoveFunction();
  void TabDestroyed();

 private:
  class WebContentsDestroyedObserver;
  ~TabsRemoveFunction() override;
  ResponseAction Run() override;
  bool RemoveTab(int tab_id, std::string* error);

  int remaining_tabs_count_ = 0;
  bool triggered_all_tab_removals_ = false;
  std::vector<std::unique_ptr<WebContentsDestroyedObserver>>
      web_contents_destroyed_observers_;
  DECLARE_EXTENSION_FUNCTION("tabs.remove", TABS_REMOVE)
};

class TabsDetectLanguageFunction
    : public ExtensionFunction,
      public content::WebContentsObserver,
      public translate::TranslateDriver::LanguageDetectionObserver {
 private:
  ~TabsDetectLanguageFunction() override {}
  ResponseAction Run() override;

  // content::WebContentsObserver:
  void NavigationEntryCommitted(
      const content::LoadCommittedDetails& load_details) override;
  void WebContentsDestroyed() override;
  void DocumentOnLoadCompletedInPrimaryMainFrame() override;

  // translate::TranslateDriver::LanguageDetectionObserver:
  void OnLanguageDetermined(
      const translate::LanguageDetectionDetails& details) override;

  // Resolves the API call with the detected |language|.
  void RespondWithLanguage(const std::string& language);

  void ExecuteDetectLanguage(content::WebContents* contents);
  void DetectLanguage(base::Value result);

  // Indicates if this instance is observing the tabs' WebContents and the
  // ContentTranslateDriver, in which case the observers must be unregistered.
  bool is_observing_ = false;

  DECLARE_EXTENSION_FUNCTION("tabs.detectLanguage", TABS_DETECTLANGUAGE)
};

class TabsCaptureVisibleTabFunction
    : public extensions::WebContentsCaptureClient,
      public ExtensionFunction {
 public:
  TabsCaptureVisibleTabFunction();

  TabsCaptureVisibleTabFunction(const TabsCaptureVisibleTabFunction&) = delete;
  TabsCaptureVisibleTabFunction& operator=(
      const TabsCaptureVisibleTabFunction&) = delete;

  // ExtensionFunction implementation.
  ResponseAction Run() override;
  void GetQuotaLimitHeuristics(
      extensions::QuotaLimitHeuristics* heuristics) const override;
  bool ShouldSkipQuotaLimiting() const override;

 protected:
  ~TabsCaptureVisibleTabFunction() override {}

 private:
  // extensions::WebContentsCaptureClient:
  ScreenshotAccess GetScreenshotAccess(
      content::WebContents* web_contents) const override;
  bool ClientAllowsTransparency() override;
  void OnCaptureSuccess(const SkBitmap& bitmap) override;
  void OnCaptureFailure(CaptureResult result) override;

  void EncodeBitmapOnWorkerThread(
      scoped_refptr<base::TaskRunner> reply_task_runner,
      const SkBitmap& bitmap);
  void OnBitmapEncodedOnUIThread(std::optional<std::string> base64_result);

  DECLARE_EXTENSION_FUNCTION("tabs.captureVisibleTab", TABS_CAPTUREVISIBLETAB)

  static std::string CaptureResultToErrorMessage(CaptureResult result);
};

// Implement API calls tabs.executeScript and tabs.insertCSS.
class ExecuteCodeInTabFunction : public extensions::ExecuteCodeFunction {
 public:
  ExecuteCodeInTabFunction();

 protected:
  ~ExecuteCodeInTabFunction() override;

  // Initializes |execute_tab_id_| and |details_|.
  extensions::ExecuteCodeFunction::InitResult Init() override;
  bool ShouldInsertCSS() const override;
  bool ShouldRemoveCSS() const override;
  bool CanExecuteScriptOnPage(std::string* error) override;
  extensions::ScriptExecutor* GetScriptExecutor(std::string* error) override;
  bool IsWebView() const override;
  int GetRootFrameId() const override;
  const GURL& GetWebViewSrc() const override;

 private:
  std::unique_ptr<extensions::ScriptExecutor> script_executor_;

  // Id of tab which executes code.
  int execute_tab_id_;
};

// Tabs
class TabsGetFunction : public ExtensionFunction {
  ~TabsGetFunction() override {}
  ResponseAction Run() override;
  DECLARE_EXTENSION_FUNCTION("tabs.get", TABS_GET)
};

class TabsGetCurrentFunction : public ExtensionFunction {
  ~TabsGetCurrentFunction() override {}
  ResponseAction Run() override;
  DECLARE_EXTENSION_FUNCTION("tabs.getCurrent", TABS_GETCURRENT)
};

class TabsUpdateFunction : public ExtensionFunction {
 public:
  TabsUpdateFunction();

 protected:
  ~TabsUpdateFunction() override {}
  bool UpdateURL(const std::string& url, int tab_id, std::string* error);
  ResponseValue GetResult();

  raw_ptr<content::WebContents, DanglingUntriaged> web_contents_;

 private:
  ResponseAction Run() override;

  DECLARE_EXTENSION_FUNCTION("tabs.update", TABS_UPDATE)
};

class TabsReloadFunction : public ExtensionFunction {
  ~TabsReloadFunction() override {}
  ResponseAction Run() override;
  DECLARE_EXTENSION_FUNCTION("tabs.reload", TABS_RELOAD)
};

class TabsExecuteScriptFunction : public ExecuteCodeInTabFunction {
 private:
  ~TabsExecuteScriptFunction() override {}

  DECLARE_EXTENSION_FUNCTION("tabs.executeScript", TABS_EXECUTESCRIPT)
};

class TabsInsertCSSFunction : public ExecuteCodeInTabFunction {
 private:
  ~TabsInsertCSSFunction() override {}

  bool ShouldInsertCSS() const override;

  DECLARE_EXTENSION_FUNCTION("tabs.insertCSS", TABS_INSERTCSS)
};

}  // namespace neva

#endif  // NEVA_EXTENSIONS_BROWSER_API_TABS_TABS_API_H_
