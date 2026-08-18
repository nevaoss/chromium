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

#include "neva/extensions/browser/api/tabs/tabs_api.h"

#include <optional>

#include "base/check_op.h"
#include "base/functional/bind.h"
#include "base/i18n/language_tag.h"
#include "base/strings/pattern.h"
#include "base/task/bind_post_task.h"
#include "base/task/sequenced_task_runner.h"
#include "base/task/thread_pool.h"
#include "base/types/optional_util.h"
#include "components/translate/core/common/language_detection_details.h"
#include "components/translate/core/language_detection/language_detection_util.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/navigation_entry.h"
#include "extensions/browser/extension_api_frame_id_map.h"
#include "extensions/browser/extensions_browser_client.h"
#include "extensions/common/extension.h"
#include "extensions/common/manifest_constants.h"
#include "extensions/common/permissions/api_permission.h"
#include "extensions/common/permissions/permissions_data.h"
#include "neva/extensions/browser/api/tabs/tabs_constants.h"
#include "neva/extensions/browser/extension_tab_util.h"
#include "neva/extensions/browser/neva_extensions_service_factory.h"
#include "neva/extensions/browser/neva_extensions_service_impl.h"
#include "neva/extensions/browser/tab_helper.h"
#include "neva/extensions/browser/web_contents_map.h"
#include "neva/extensions/common/api/tabs.h"
#include "neva/extensions/common/api/windows.h"
#include "third_party/skia/include/core/SkBitmap.h"

namespace neva {

namespace windows = extensions::api::windows;
namespace tabs = extensions::api::tabs;

using extensions::api::extension_types::InjectDetails;

// |error_message| can optionally be passed in and will be set with an
// appropriate message if the tab cannot be found by id.
bool GetTabById(int tab_id,
                content::BrowserContext* context,
                content::WebContents** contents,
                std::string* error_message) {
  if (tab_id < 1) {
    if (error_message) {
      *error_message = extensions::ErrorUtils::FormatErrorMessage(
          tabs_constants::kTabNotFoundError, base::NumberToString(tab_id));
    }
    return false;
  }

  WebContentsMap* web_contents_map = WebContentsMap::GetInstance();
  for (auto& it : *web_contents_map) {
    content::WebContents* web_contents = it.second;

    uint64_t found_tab_id = NevaExtensionsServiceFactory::GetService(context)
                                ->GetTabHelper()
                                ->GetTabIdFromWebContents(web_contents);
    if (tab_id == static_cast<int>(found_tab_id)) {
      *contents = web_contents;
      return true;
    }
  }

  if (error_message) {
    *error_message = extensions::ErrorUtils::FormatErrorMessage(
        tabs_constants::kTabNotFoundError, base::NumberToString(tab_id));
  }

  return false;
}

// Returns true if either |boolean| is disengaged, or if |boolean| and
// |value| are equal. This function is used to check if a tab's parameters match
// those of the browser.
bool MatchesBool(const std::optional<bool>& boolean, bool value) {
  return !boolean || *boolean == value;
}

base::DictValue getCurrentTab(content::BrowserContext* browser_context) {
  base::DictValue dict;

  WebContentsMap* web_contents_map = WebContentsMap::GetInstance();
  for (auto& it : *web_contents_map) {
    content::WebContents* web_contents = it.second;

    uint64_t tab_id = NevaExtensionsServiceFactory::GetService(browser_context)
                          ->GetTabHelper()
                          ->GetTabIdFromWebContents(web_contents);
    if (tab_id == 0) {
      continue;
    }

    if (web_contents->GetVisibility() != content::Visibility::HIDDEN) {
      dict = ExtensionTabUtil::CreateWindowValueForExtension(
          static_cast<int>(tab_id), true);
      break;
    }
  }

  return dict;
}

content::WebContents* GetWebContentsOfCurrentTab(
    content::BrowserContext* browser_context) {
  WebContentsMap* web_contents_map = WebContentsMap::GetInstance();
  for (auto& it : *web_contents_map) {
    content::WebContents* web_contents = it.second;

    uint64_t tab_id = NevaExtensionsServiceFactory::GetService(browser_context)
                          ->GetTabHelper()
                          ->GetTabIdFromWebContents(web_contents);
    if (tab_id == 0) {
      continue;
    }

    if (web_contents->GetVisibility() != content::Visibility::HIDDEN) {
      return web_contents;
    }
  }

  return nullptr;
}

// Windows ---------------------------------------------------------------------

ExtensionFunction::ResponseAction WindowsGetCurrentFunction::Run() {
  base::DictValue dict = getCurrentTab(browser_context());
  if (dict.empty()) {
    return RespondNow(Error("No current window"));
  }

  return RespondNow(WithArguments(std::move(dict)));
}

ExtensionFunction::ResponseAction WindowsGetLastFocusedFunction::Run() {
  base::DictValue dict = getCurrentTab(browser_context());
  if (dict.empty()) {
    return RespondNow(Error("No current window"));
  }

  return RespondNow(WithArguments(std::move(dict)));
}

ExtensionFunction::ResponseAction WindowsGetAllFunction::Run() {
  std::optional<windows::GetAll::Params> params =
      windows::GetAll::Params::Create(args());
  EXTENSION_FUNCTION_VALIDATE(params);

  WebContentsMap* web_contents_map = WebContentsMap::GetInstance();
  base::ListValue window_list;

  for (auto& it : *web_contents_map) {
    content::WebContents* web_contents = it.second;

    uint64_t tab_id =
        NevaExtensionsServiceFactory::GetService(browser_context())
            ->GetTabHelper()
            ->GetTabIdFromWebContents(web_contents);
    if (tab_id == 0) {
      continue;
    }

    auto window_type =
        NevaExtensionsServiceFactory::GetService(browser_context())
            ->GetTabHelper()
            ->GetExtensionWindowType(tab_id);
    if (window_type) {
      bool focused =
          web_contents->GetVisibility() != content::Visibility::HIDDEN;
      window_list.Append(ExtensionTabUtil::CreateWindowValueForExtension(
          tab_id, focused,
          static_cast<windows::WindowType>(window_type.value())));
    }
  }

  return RespondNow(WithArguments(std::move(window_list)));
}

ExtensionFunction::ResponseAction WindowsCreateFunction::Run() {
  std::optional<windows::Create::Params> params =
      windows::Create::Params::Create(args());
  EXTENSION_FUNCTION_VALIDATE(params);

  if (!params->create_data || !params->create_data->url.has_value()) {
    return RespondNow(Error("We don't support missing url yet."));
  }

  if (params->create_data->type != windows::CreateType::kPopup &&
      params->create_data->type != windows::CreateType::kNormal) {
    return RespondNow(Error("Invalid window type."));
  }

  GURL url = GURL(params->create_data->url->as_string.value());
  if (!url.is_valid() && extension()) {
    url = extension()->ResolveExtensionURL(
        params->create_data->url->as_string.value());
  }

  std::string tab_type = NevaExtensionsServiceFactory::GetService(browser_context())
                     ->GetTabHelper()
                     ->GetBrowserTabType(params->create_data->type);

  NevaExtensionsServiceFactory::GetService(browser_context())
      ->OnExtensionTabCreationRequested(
          url.spec(), tab_type,
          base::BindOnce(&WindowsCreateFunction::OnWindowsCreated, this));
  return RespondLater();
}

void WindowsCreateFunction::OnWindowsCreated(int window_id) {
  if (has_callback()) {
    Respond(WithArguments(
        ExtensionTabUtil::CreateWindowValueForExtension(window_id, true)));
    return;
  }
  Respond(NoArguments());
}

ExtensionFunction::ResponseAction WindowsUpdateFunction::Run() {
  std::optional<windows::Update::Params> params =
      windows::Update::Params::Create(args());
  EXTENSION_FUNCTION_VALIDATE(params);

  if (params->update_info.focused) {
    NevaExtensionsServiceFactory::GetService(browser_context())
        ->OnExtensionTabFocusRequested(
            static_cast<uint64_t>(params->window_id));
  }

  return RespondNow(
      WithArguments(ExtensionTabUtil::CreateWindowValueForExtension(
          params->window_id, true)));
}

ExtensionFunction::ResponseAction WindowsRemoveFunction::Run() {
  std::optional<windows::Remove::Params> params =
      windows::Remove::Params::Create(args());
  EXTENSION_FUNCTION_VALIDATE(params);

  NevaExtensionsServiceFactory::GetService(browser_context())
      ->OnExtensionTabCloseRequested(static_cast<uint64_t>(params->window_id));

  return RespondNow(NoArguments());
}

// Tabs ------------------------------------------------------------------------

ExtensionFunction::ResponseAction TabsCreateFunction::Run() {
  std::optional<tabs::Create::Params> params =
      tabs::Create::Params::Create(args());
  if (!params->create_properties.url)
    return RespondNow(Error("We don't support missing url yet."));

  std::string tab_type = NevaExtensionsServiceFactory::GetService(browser_context())
                     ->GetTabHelper()
                     ->GetBrowserNormalTabType();

  NevaExtensionsServiceFactory::GetService(browser_context())
      ->OnExtensionTabCreationRequested(
          *(params->create_properties.url), tab_type,
          base::BindOnce(&TabsCreateFunction::OnTabCreated, this));
  return RespondLater();
}

void TabsCreateFunction::OnTabCreated(int tab_id) {
  if (has_callback()) {
    tabs::Tab tab_object;
    tab_object.id = tab_id;
    Respond(WithArguments(tab_object.ToValue()));
    return;
  }
  Respond(NoArguments());
}

ExtensionFunction::ResponseAction TabsQueryFunction::Run() {
  std::optional<tabs::Query::Params> params =
      tabs::Query::Params::Create(args());
  EXTENSION_FUNCTION_VALIDATE(params);

  extensions::URLPatternSet url_patterns;
  if (params->query_info.url) {
    std::vector<std::string> url_pattern_strings;
    if (params->query_info.url->as_string) {
      url_pattern_strings.push_back(*params->query_info.url->as_string);
    } else if (params->query_info.url->as_strings) {
      url_pattern_strings.swap(*params->query_info.url->as_strings);
    }
    // It is o.k. to use URLPattern::SCHEME_ALL here because this function does
    // not grant access to the content of the tabs, only to seeing their URLs
    // and meta data.
    std::string error;
    if (!url_patterns.Populate(url_pattern_strings, URLPattern::SCHEME_ALL,
                               true, &error)) {
      return RespondNow(Error(std::move(error)));
    }
  }

  std::string title = params->query_info.title.value_or(std::string());

  WebContentsMap* web_contents_map = WebContentsMap::GetInstance();
  base::ListValue result;

  for (auto& it : *web_contents_map) {
    content::WebContents* web_contents = it.second;

    uint64_t tab_id =
        NevaExtensionsServiceFactory::GetService(browser_context())
            ->GetTabHelper()
            ->GetTabIdFromWebContents(web_contents);
    if (tab_id == 0) {
      continue;
    }

    if (!MatchesBool(
            params->query_info.active,
            web_contents->GetVisibility() != content::Visibility::HIDDEN)) {
      continue;
    }

    if (!title.empty() || !url_patterns.is_empty()) {
      if (!title.empty() && !base::MatchPattern(web_contents->GetTitle(),
                                                base::UTF8ToUTF16(title))) {
        continue;
      }

      if (!url_patterns.is_empty() &&
          !url_patterns.MatchesURL(web_contents->GetURL())) {
        continue;
      }
    }

    extensions::api::tabs::Tab tab_object;
    tab_object.id = tab_id;
    tab_object.active =
        (web_contents->GetVisibility() != content::Visibility::HIDDEN);
    tab_object.selected = tab_object.active;
    tab_object.url = web_contents->GetLastCommittedURL().spec();
    tab_object.title = base::UTF16ToUTF8(web_contents->GetTitle());
    tab_object.group_id = -1;
    tab_object.incognito = web_contents->GetBrowserContext()->IsOffTheRecord();
    tab_object.highlighted = tab_object.active;
    result.Append(tab_object.ToValue());
  }

  return RespondNow(WithArguments(std::move(result)));
}

ExtensionFunction::ResponseAction TabsGetFunction::Run() {
  std::optional<tabs::Get::Params> params = tabs::Get::Params::Create(args());
  EXTENSION_FUNCTION_VALIDATE(params);
  int tab_id = params->tab_id;

  content::WebContents* web_contents = nullptr;
  std::string error;
  if (!GetTabById(tab_id, browser_context(), &web_contents, &error)) {
    return RespondNow(Error(std::move(error)));
  }

  extensions::api::tabs::Tab tab_object;
  tab_object.id = tab_id;
  tab_object.window_id = tab_id;
  tab_object.active =
      (web_contents->GetVisibility() != content::Visibility::HIDDEN);
  tab_object.selected = tab_object.active;
  tab_object.url = web_contents->GetLastCommittedURL().spec();
  tab_object.title = base::UTF16ToUTF8(web_contents->GetTitle());
  tab_object.group_id = -1;
  tab_object.incognito = web_contents->GetBrowserContext()->IsOffTheRecord();
  tab_object.highlighted = tab_object.active;
  base::DictValue result{tab_object.ToValue()};
  return RespondNow(WithArguments(std::move(result)));
}

ExtensionFunction::ResponseAction TabsGetCurrentFunction::Run() {
  DCHECK(dispatcher());

  // Return the caller, if it's a tab. If not the result isn't an error but an
  // empty tab (hence returning true).
  content::WebContents* caller_contents = GetSenderWebContents();
  if (caller_contents && ExtensionTabUtil::GetTabId(caller_contents) > 0) {
    return RespondNow(ArgumentList(tabs::Get::Results::Create(
        ExtensionTabUtil::CreateTabObject(caller_contents))));
  }
  return RespondNow(NoArguments());
}

TabsUpdateFunction::TabsUpdateFunction() : web_contents_(nullptr) {}

ExtensionFunction::ResponseAction TabsUpdateFunction::Run() {
  std::optional<tabs::Update::Params> params =
      tabs::Update::Params::Create(args());
  EXTENSION_FUNCTION_VALIDATE(params);

  int tab_id = -1;
  content::WebContents* contents = nullptr;
  if (!params->tab_id) {
    contents = GetWebContentsOfCurrentTab(browser_context());
    if (!contents) {
      return RespondNow(Error(tabs_constants::kNoSelectedTabError));
    }
    tab_id = ExtensionTabUtil::GetTabId(contents);
  } else {
    tab_id = *params->tab_id;
  }

  std::string error;
  if (!GetTabById(tab_id, browser_context(), &contents, &error)) {
    return RespondNow(Error(std::move(error)));
  }

  web_contents_ = contents;

  bool active = false;
  // TODO(rafaelw): Setting |active| from js doesn't make much sense.
  // Move tab selection management up to window.
  if (params->update_properties.selected) {
    active = *params->update_properties.selected;
  }

  // The 'active' property has replaced 'selected'.
  if (params->update_properties.active) {
    active = *params->update_properties.active;
  }

  if (active) {
    NevaExtensionsServiceFactory::GetService(browser_context())
        ->OnExtensionTabFocusRequested(static_cast<uint64_t>(tab_id));
  }

  // TODO(neva): Not supported properties
  // - auto_discardable
  // - highlighted
  // - muted
  // - opener_tab_id
  // - pinned

  // TODO(rafaelw): handle setting remaining tab properties:
  // -title
  // -favIconUrl

  // Navigate the tab to a new location if the url is different.
  if (params->update_properties.url) {
    std::string updated_url = *params->update_properties.url;
    if (!UpdateURL(updated_url, tab_id, &error)) {
      return RespondNow(Error(std::move(error)));
    }
  }

  return RespondNow(GetResult());
}

bool TabsUpdateFunction::UpdateURL(const std::string& url_string,
                                   int tab_id,
                                   std::string* error) {
  auto url = ExtensionTabUtil::PrepareURLForNavigation(url_string, extension(),
                                                       browser_context());
  if (!url.has_value()) {
    *error = std::move(url.error());
    return false;
  }

  content::NavigationController::LoadURLParams load_params(*url);

  // Treat extension-initiated navigations as renderer-initiated so that the URL
  // does not show in the omnibox until it commits.  This avoids URL spoofs
  // since URLs can be opened on behalf of untrusted content.
  load_params.is_renderer_initiated = true;
  // All renderer-initiated navigations need to have an initiator origin.
  load_params.initiator_origin = extension()->origin();
  // |source_site_instance| needs to be set so that a renderer process
  // compatible with |initiator_origin| is picked by Site Isolation.
  load_params.source_site_instance = content::SiteInstance::CreateForURL(
      web_contents_->GetBrowserContext(),
      load_params.initiator_origin->GetURL());

  // Marking the navigation as initiated via an API means that the focus
  // will stay in the omnibox - see https://crbug.com/1085779.
  load_params.transition_type = ui::PAGE_TRANSITION_FROM_API;

  web_contents_->GetController().LoadURLWithParams(load_params);

  DCHECK_EQ(*url,
            web_contents_->GetController().GetPendingEntry()->GetVirtualURL());

  return true;
}

ExtensionFunction::ResponseValue TabsUpdateFunction::GetResult() {
  if (!has_callback()) {
    return NoArguments();
  }

  return ArgumentList(tabs::Get::Results::Create(
      ExtensionTabUtil::CreateTabObject(web_contents_)));
}

ExtensionFunction::ResponseAction TabsReloadFunction::Run() {
  std::optional<tabs::Reload::Params> params =
      tabs::Reload::Params::Create(args());
  EXTENSION_FUNCTION_VALIDATE(params);

  bool bypass_cache = false;
  if (params->reload_properties && params->reload_properties->bypass_cache) {
    bypass_cache = *params->reload_properties->bypass_cache;
  }

  content::WebContents* web_contents = nullptr;
  if (!params->tab_id) {
    web_contents = GetWebContentsOfCurrentTab(browser_context());
    if (!web_contents) {
      return RespondNow(Error(tabs_constants::kNoCurrentTabError));
    }
  } else {
    int tab_id = *params->tab_id;

    std::string error;
    if (!GetTabById(tab_id, browser_context(), &web_contents, &error)) {
      return RespondNow(Error(std::move(error)));
    }
  }

  web_contents->GetController().Reload(
      bypass_cache ? content::ReloadType::BYPASSING_CACHE
                   : content::ReloadType::NORMAL,
      true);

  return RespondNow(NoArguments());
}

class TabsRemoveFunction::WebContentsDestroyedObserver
    : public content::WebContentsObserver {
 public:
  WebContentsDestroyedObserver(TabsRemoveFunction* owner,
                               content::WebContents* watched_contents)
      : content::WebContentsObserver(watched_contents), owner_(owner) {}

  ~WebContentsDestroyedObserver() override = default;
  WebContentsDestroyedObserver(const WebContentsDestroyedObserver&) = delete;
  WebContentsDestroyedObserver& operator=(const WebContentsDestroyedObserver&) =
      delete;

  // WebContentsObserver
  void WebContentsDestroyed() override { owner_->TabDestroyed(); }

 private:
  // Guaranteed to outlive this object.
  raw_ptr<TabsRemoveFunction> owner_;
};

TabsRemoveFunction::TabsRemoveFunction() = default;
TabsRemoveFunction::~TabsRemoveFunction() = default;

ExtensionFunction::ResponseAction TabsRemoveFunction::Run() {
  std::optional<tabs::Remove::Params> params =
      tabs::Remove::Params::Create(args());
  EXTENSION_FUNCTION_VALIDATE(params);

  std::string error;
  if (params->tab_ids.as_integers) {
    std::vector<int>& tab_ids = *params->tab_ids.as_integers;
    for (int tab_id : tab_ids) {
      if (!RemoveTab(tab_id, &error)) {
        return RespondNow(Error(std::move(error)));
      }
    }
  } else {
    EXTENSION_FUNCTION_VALIDATE(params->tab_ids.as_integer);
    if (!RemoveTab(*params->tab_ids.as_integer, &error)) {
      return RespondNow(Error(std::move(error)));
    }
  }
  triggered_all_tab_removals_ = true;
  DCHECK(!did_respond());
  // WebContentsDestroyed will return the response in most cases, except when
  // the last tab closed immediately (it won't return a response because
  // |triggered_all_tab_removals_| will still be false). In this case we should
  // return the response from here.
  if (remaining_tabs_count_ == 0) {
    return RespondNow(NoArguments());
  }
  return RespondLater();
}

bool TabsRemoveFunction::RemoveTab(int tab_id, std::string* error) {
  content::WebContents* contents = nullptr;
  if (!GetTabById(tab_id, browser_context(), &contents, error)) {
    return false;
  }

  // The tab might not immediately close after calling Close() below, so we
  // should wait until WebContentsDestroyed is called before responding.
  web_contents_destroyed_observers_.push_back(
      std::make_unique<WebContentsDestroyedObserver>(this, contents));
  // Ensure that we're going to keep this class alive until
  // |remaining_tabs_count| reaches zero. This relies on WebContents::Close()
  // always (eventually) resulting in a WebContentsDestroyed() call; otherwise,
  // this function will never respond and may leak.
  AddRef();
  remaining_tabs_count_++;

  // There's a chance that the tab is being dragged, or we're in some other
  // nested event loop. This code path ensures that the tab is safely closed
  // under such circumstances, whereas |TabStripModel::CloseWebContentsAt()|
  // does not.
  contents->Close();
  return true;
}

void TabsRemoveFunction::TabDestroyed() {
  DCHECK_GT(remaining_tabs_count_, 0);
  // One of the tabs we wanted to remove had been destroyed.
  remaining_tabs_count_--;
  // If we've triggered all the tab removals we need, and this is the last tab
  // we're waiting for and we haven't sent a response (it's possible that we've
  // responded earlier in case of errors, etc.), send a response.
  if (triggered_all_tab_removals_ && remaining_tabs_count_ == 0 &&
      !did_respond()) {
    Respond(NoArguments());
  }
  Release();
}

TabsCaptureVisibleTabFunction::TabsCaptureVisibleTabFunction() {}

base::expected<void, extensions::ScreenshotAccessError>
TabsCaptureVisibleTabFunction::GetScreenshotAccess(
    content::WebContents* web_contents) const {
  return extensions::ExtensionsBrowserClient::Get()->IsScreenshotRestricted(web_contents);
}

bool TabsCaptureVisibleTabFunction::ClientAllowsTransparency() {
  return false;
}

ExtensionFunction::ResponseAction TabsCaptureVisibleTabFunction::Run() {
  using extensions::api::extension_types::ImageDetails;

  EXTENSION_FUNCTION_VALIDATE(has_args());
  std::optional<ImageDetails> image_details;
  if (args().size() > 1) {
    image_details = ImageDetails::FromValue(args()[1]);
  }

  content::WebContents* contents =
      GetWebContentsOfCurrentTab(browser_context());
  if (!contents) {
    return RespondNow(Error(tabs_constants::kNoActiveWebContentsToCapture));
  }

  // NOTE: CaptureAsync() may invoke its callback from a background thread,
  // hence the BindPostTask().
  const CaptureResult capture_result = CaptureAsync(
      contents, base::OptionalToPtr(image_details),
      base::BindPostTaskToCurrentDefault(base::BindOnce(
          &TabsCaptureVisibleTabFunction::CopyFromSurfaceComplete, this)));
  if (capture_result == OK) {
    // CopyFromSurfaceComplete might have already responded.
    return did_respond() ? AlreadyResponded() : RespondLater();
  }

  return RespondNow(Error(CaptureResultToErrorMessage(capture_result)));
}

void TabsCaptureVisibleTabFunction::GetQuotaLimitHeuristics(
    extensions::QuotaLimitHeuristics* heuristics) const {
  constexpr base::TimeDelta kSecond = base::Seconds(1);
  extensions::QuotaLimitHeuristic::Config limit = {
      tabs::MAX_CAPTURE_VISIBLE_TAB_CALLS_PER_SECOND, kSecond};

  heuristics->push_back(std::make_unique<extensions::QuotaService::TimedLimit>(
      limit,
      std::make_unique<
          extensions::QuotaLimitHeuristic::SingletonBucketMapper>(),
      "MAX_CAPTURE_VISIBLE_TAB_CALLS_PER_SECOND"));
}

bool TabsCaptureVisibleTabFunction::ShouldSkipQuotaLimiting() const {
  return false;
}

void TabsCaptureVisibleTabFunction::OnCaptureSuccess(const SkBitmap& bitmap) {
  base::ThreadPool::PostTask(
      FROM_HERE, {base::TaskPriority::USER_VISIBLE},
      base::BindOnce(&TabsCaptureVisibleTabFunction::EncodeBitmapOnWorkerThread,
                     this, base::SingleThreadTaskRunner::GetCurrentDefault(),
                     bitmap));
}

void TabsCaptureVisibleTabFunction::EncodeBitmapOnWorkerThread(
    scoped_refptr<base::TaskRunner> reply_task_runner,
    const SkBitmap& bitmap) {
  std::optional<std::string> base64_result = EncodeBitmap(bitmap);
  reply_task_runner->PostTask(
      FROM_HERE,
      base::BindOnce(&TabsCaptureVisibleTabFunction::OnBitmapEncodedOnUIThread,
                     this, std::move(base64_result)));
}

void TabsCaptureVisibleTabFunction::OnBitmapEncodedOnUIThread(
    std::optional<std::string> base64_result) {
  if (!base64_result.has_value()) {
    OnCaptureFailure(FAILURE_REASON_ENCODING_FAILED);
    return;
  }

  Respond(WithArguments(std::move(base64_result.value())));
}

void TabsCaptureVisibleTabFunction::OnCaptureFailure(CaptureResult result) {
  Respond(Error(CaptureResultToErrorMessage(result)));
}

// static.
std::string TabsCaptureVisibleTabFunction::CaptureResultToErrorMessage(
    CaptureResult result) {
  const char* reason_description = "internal error";
  switch (result) {
    case FAILURE_REASON_READBACK_FAILED:
      reason_description = "image readback failed";
      break;
    case FAILURE_REASON_ENCODING_FAILED:
      reason_description = "encoding failed";
      break;
    case FAILURE_REASON_VIEW_INVISIBLE:
      reason_description = "view is invisible";
      break;
    case FAILURE_REASON_SCREEN_SHOTS_DISABLED:
      return tabs_constants::kScreenshotsDisabled;
    case FAILURE_REASON_SCREEN_SHOTS_DISABLED_BY_DLP:
      return tabs_constants::kScreenshotsDisabledByDlp;
    case OK:
      NOTREACHED() << "CaptureResultToErrorMessage should not be called"
                      " with a successful result";
  }
  return extensions::ErrorUtils::FormatErrorMessage("Failed to capture tab: *",
                                                    reason_description);
}

ExtensionFunction::ResponseAction TabsDetectLanguageFunction::Run() {
  std::optional<tabs::DetectLanguage::Params> params =
      tabs::DetectLanguage::Params::Create(args());
  EXTENSION_FUNCTION_VALIDATE(params);

  int tab_id = 0;
  content::WebContents* contents = nullptr;

  // If |tab_id| is specified, look for it. Otherwise default to selected tab
  // in the current window.
  std::string error;
  if (params->tab_id) {
    tab_id = *params->tab_id;
    if (!GetTabById(tab_id, browser_context(), &contents, &error)) {
      return RespondNow(Error(std::move(error)));
    }
  } else {
    contents = GetWebContentsOfCurrentTab(browser_context());
    if (!contents) {
      return RespondNow(Error(tabs_constants::kNoSelectedTabError));
    }
  }

  if (contents->GetController().NeedsReload()) {
    // If the tab hasn't been loaded, don't wait for the tab to load.
    return RespondNow(
        Error(tabs_constants::kCannotDetermineLanguageOfUnloadedTab));
  }

  AddRef();  // Balanced in RespondWithLanguage().

  // The tab contents does not know its language yet. Let's wait until it
  // receives it, or until the tab is closed/navigates to some other page.

  // Observe the WebContents' lifetime and navigations.
  Observe(contents);
  // Wait until the language is determined.
  if (contents->IsDocumentOnLoadCompletedInPrimaryMainFrame()) {
    base::SequencedTaskRunner::GetCurrentDefault()->PostTask(
        FROM_HERE,
        base::BindOnce(&TabsDetectLanguageFunction::ExecuteDetectLanguage, this,
                       contents));
  }
  is_observing_ = true;

  return RespondLater();
}

void TabsDetectLanguageFunction::ExecuteDetectLanguage(
    content::WebContents* contents) {
  contents->GetPrimaryMainFrame()->ExecuteJavaScript(
      u"document.body.innerText",
      base::BindOnce(&TabsDetectLanguageFunction::DetectLanguage, this));
}

void TabsDetectLanguageFunction::DetectLanguage(base::Value result) {
  std::string text = result.GetString();
  bool is_model_reliable = false;
  float model_reliability_score = 0.0;
  base::i18n::LanguageTag detected_language_tag = translate::DetermineTextLanguage(
      text, &is_model_reliable, model_reliability_score)
      .value_or(base::i18n::GetKnownLanguageTag("und"));;

  translate::LanguageDetectionDetails details;
  details.adopted_language = std::string(detected_language_tag.tag_string());
  OnLanguageDetermined(details);
}

void TabsDetectLanguageFunction::NavigationEntryCommitted(
    const content::LoadCommittedDetails& load_details) {
  // Call RespondWithLanguage() with an empty string as we want to guarantee the
  // callback is called for every API call the extension made.
  RespondWithLanguage(std::string());
}

void TabsDetectLanguageFunction::WebContentsDestroyed() {
  // Call RespondWithLanguage() with an empty string as we want to guarantee the
  // callback is called for every API call the extension made.
  RespondWithLanguage(std::string());
}

void TabsDetectLanguageFunction::DocumentOnLoadCompletedInPrimaryMainFrame() {
  base::SequencedTaskRunner::GetCurrentDefault()->PostTask(
      FROM_HERE,
      base::BindOnce(&TabsDetectLanguageFunction::ExecuteDetectLanguage, this,
                     web_contents()));
}

void TabsDetectLanguageFunction::OnLanguageDetermined(
    const translate::LanguageDetectionDetails& details) {
  RespondWithLanguage(details.adopted_language);
}

void TabsDetectLanguageFunction::RespondWithLanguage(
    const std::string& language) {
  // Stop observing.
  if (is_observing_) {
    Observe(nullptr);
  }

  Respond(WithArguments(language));
  Release();  // Balanced in Run()
}

ExecuteCodeInTabFunction::ExecuteCodeInTabFunction() : execute_tab_id_(0) {}

ExecuteCodeInTabFunction::~ExecuteCodeInTabFunction() {}

extensions::ExecuteCodeFunction::InitResult ExecuteCodeInTabFunction::Init() {
  if (init_result_) {
    return init_result_.value();
  }

  if (args().size() < 2) {
    return set_init_result(VALIDATION_FAILURE);
  }

  const auto& tab_id_value = args()[0];
  // |tab_id| is optional so it's ok if it's not there.
  int tab_id = 0;
  if (tab_id_value.is_int()) {
    // But if it is present, it needs to be positive.
    tab_id = tab_id_value.GetInt();
    if (tab_id < 1) {
      return set_init_result(VALIDATION_FAILURE);
    }
  }

  // |details| are not optional.
  const base::Value& details_value = args()[1];
  if (!details_value.is_dict()) {
    return set_init_result(VALIDATION_FAILURE);
  }
  auto details = InjectDetails::FromValue(details_value.GetDict());
  if (!details) {
    return set_init_result(VALIDATION_FAILURE);
  }

  // If the tab ID wasn't given then it needs to be converted to the
  // currently active tab's ID.
  if (tab_id == 0) {
    WebContentsMap* web_contents_map = WebContentsMap::GetInstance();
    for (auto& it : *web_contents_map) {
      content::WebContents* web_contents = it.second;
      uint64_t contents_tab_id =
          NevaExtensionsServiceFactory::GetService(browser_context())
              ->GetTabHelper()
              ->GetTabIdFromWebContents(web_contents);
      if (contents_tab_id == 0) {
        continue;
      }
      if (web_contents->GetVisibility() != content::Visibility::HIDDEN) {
        tab_id = contents_tab_id;
      }
    }
    CHECK_GE(tab_id, 0);
  }

  execute_tab_id_ = tab_id;
  details_ = std::move(details);
  set_host_id(extensions::mojom::HostID(
      extensions::mojom::HostID::HostType::kExtensions, extension()->id()));
  return set_init_result(SUCCESS);
}

bool ExecuteCodeInTabFunction::ShouldInsertCSS() const {
  return false;
}

bool ExecuteCodeInTabFunction::ShouldRemoveCSS() const {
  return false;
}

bool ExecuteCodeInTabFunction::CanExecuteScriptOnPage(std::string* error) {
  content::WebContents* contents = nullptr;

  // If |tab_id| is specified, look for the tab. Otherwise default to selected
  // tab in the current window.
  CHECK_GE(execute_tab_id_, 1);
  contents = NevaExtensionsServiceFactory::GetService(browser_context())
                 ->GetTabHelper()
                 ->GetWebContentsFromId(execute_tab_id_);
  if (!contents) {
    return false;
  }

  int frame_id = details_->frame_id
                     ? *details_->frame_id
                     : extensions::ExtensionApiFrameIdMap::kTopFrameId;
  content::RenderFrameHost* render_frame_host =
      extensions::ExtensionApiFrameIdMap::GetRenderFrameHostById(contents,
                                                                 frame_id);
  if (!render_frame_host) {
    *error = extensions::ErrorUtils::FormatErrorMessage(
        tabs_constants::kFrameNotFoundError, base::NumberToString(frame_id),
        base::NumberToString(execute_tab_id_));
    return false;
  }

  // Content scripts declared in manifest.json can access frames at about:-URLs
  // if the extension has permission to access the frame's origin, so also allow
  // programmatic content scripts at about:-URLs for allowed origins.
  GURL effective_document_url(render_frame_host->GetLastCommittedURL());
  bool is_about_url = effective_document_url.SchemeIs(url::kAboutScheme);
  if (is_about_url && details_->match_about_blank &&
      *details_->match_about_blank) {
    effective_document_url =
        GURL(render_frame_host->GetLastCommittedOrigin().Serialize());
  }

  if (!effective_document_url.is_valid()) {
    // Unknown URL, e.g. because no load was committed yet. Allow for now, the
    // renderer will check again and fail the injection if needed.
    return true;
  }

  // NOTE: This can give the wrong answer due to race conditions, but it is OK,
  // we check again in the renderer.
  if (!extension()->permissions_data()->CanAccessPage(effective_document_url,
                                                      execute_tab_id_, error)) {
    if (is_about_url &&
        extension()->permissions_data()->active_permissions().HasAPIPermission(
            extensions::mojom::APIPermissionID::kTab)) {
      *error = extensions::ErrorUtils::FormatErrorMessage(
          extensions::manifest_errors::kCannotAccessAboutUrl,
          render_frame_host->GetLastCommittedURL().spec(),
          render_frame_host->GetLastCommittedOrigin().Serialize());
    }
    return false;
  }

  return true;
}

extensions::ScriptExecutor* ExecuteCodeInTabFunction::GetScriptExecutor(
    std::string* error) {
  content::WebContents* contents =
      NevaExtensionsServiceFactory::GetService(browser_context())
          ->GetTabHelper()
          ->GetWebContentsFromId(execute_tab_id_);

  if (!contents) {
    return nullptr;
  }

  return WebContentsMap::GetInstance()->GetScriptExecutor(contents);
}

bool ExecuteCodeInTabFunction::IsWebView() const {
  return false;
}

int ExecuteCodeInTabFunction::GetRootFrameId() const {
  return extensions::ExtensionApiFrameIdMap::kTopFrameId;
}

const GURL& ExecuteCodeInTabFunction::GetWebViewSrc() const {
  return GURL::EmptyGURL();
}

bool TabsInsertCSSFunction::ShouldInsertCSS() const {
  return true;
}

}  // namespace neva
