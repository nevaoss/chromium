// Copyright 2023 LG Electronics, Inc.
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

#include "neva/extensions/browser/extension_tab_util.h"

#include "base/containers/fixed_flat_set.h"
#include "base/json/json_reader.h"
#include "base/logging.h"
#include "components/url_formatter/url_fixer.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/url_constants.h"
#include "extensions/browser/event_router.h"
#include "extensions/browser/extension_util.h"
#include "extensions/common/error_utils.h"
#include "extensions/common/extension_features.h"
#include "extensions/common/permissions/api_permission.h"
#include "extensions/common/permissions/permissions_data.h"
#include "neva/extensions/browser/api/tabs/tabs_constants.h"
#include "neva/extensions/browser/neva_extensions_service_factory.h"
#include "neva/extensions/browser/neva_extensions_service_impl.h"
#include "neva/extensions/browser/tab_helper.h"
#include "neva/extensions/common/api/tabs.h"
#include "third_party/blink/public/common/chrome_debug_urls.h"
#include "url/gurl.h"

namespace neva {

namespace tabs = extensions::api::tabs;
namespace windows = extensions::api::windows;

namespace {

bool IsFileUrl(const GURL& url) {
  return url.SchemeIsFile() || (url.SchemeIs(content::kViewSourceScheme) &&
                                GURL(url.GetContent()).SchemeIsFile());
}

bool HasValidMainFrameProcess(content::WebContents* contents) {
  content::RenderFrameHost* main_frame_host = contents->GetPrimaryMainFrame();
  content::RenderProcessHost* process_host = main_frame_host->GetProcess();
  return process_host->IsReady() && process_host->IsInitializedAndNotDead();
}

}  // namespace

// TODO(neva): There is no way to distinguish among types (normal, popup, etc.)
// at the moment, however, the chrome.windows.create only allows to create popup
// type, so will return as popup type for now.
base::DictValue ExtensionTabUtil::CreateWindowValueForExtension(
    int window_id,
    bool focused,
    windows::WindowType type,
    bool always_on_top,
    bool incognito) {
  base::DictValue dict;
  dict.Set("id", window_id);
  dict.Set("alwaysOnTop", always_on_top);
  dict.Set("focused", focused);
  dict.Set("incognito", incognito);
  dict.Set("left", 0);
  dict.Set("top", 0);
  dict.Set("width", 0);

  std::string type_str;
  switch (type) {
    case windows::WindowType::kPopup:
      type_str = "popup";
      break;
    case windows::WindowType::kPanel:
      type_str = "panel";
      break;
    case windows::WindowType::kApp:
      type_str = "app";
      break;
    case windows::WindowType::kDevtools:
      type_str = "devtools";
      break;
    case windows::WindowType::kNormal:
    default:
      type_str = "normal";
      break;
  }
  dict.Set("type", type_str);

  return dict;
}

int ExtensionTabUtil::GetTabId(content::WebContents* web_contents) {
  uint64_t tab_id = NevaExtensionsServiceFactory::GetService(
                        web_contents->GetBrowserContext())
                        ->GetTabHelper()
                        ->GetTabIdFromWebContents(web_contents);
  return static_cast<int>(tab_id);
}

extensions::api::tabs::Tab ExtensionTabUtil::CreateTabObject(
    content::WebContents* contents) {
  int tab_id = GetTabId(contents);

  extensions::api::tabs::Tab tab_object;
  tab_object.id = tab_id;
  tab_object.window_id = tab_id;
  tab_object.status = GetLoadingStatus(contents);
  tab_object.active =
      (contents->GetVisibility() != content::Visibility::HIDDEN);
  tab_object.selected = tab_object.active;
  tab_object.url = contents->GetLastCommittedURL().spec();
  content::NavigationEntry* pending_entry =
      contents->GetController().GetPendingEntry();
  if (pending_entry) {
    tab_object.pending_url = pending_entry->GetVirtualURL().spec();
  }
  tab_object.title = base::UTF16ToUTF8(contents->GetTitle());
  tab_object.group_id = -1;
  tab_object.incognito = contents->GetBrowserContext()->IsOffTheRecord();
  tab_object.highlighted = tab_object.active;
  return tab_object;
}

GURL ExtensionTabUtil::ResolvePossiblyRelativeURL(
    const std::string& url_string,
    const extensions::Extension* extension) {
  GURL url = GURL(url_string);
  if (!url.is_valid() && extension) {
    url = extension->ResolveExtensionURL(url_string);
  }

  return url;
}

bool ExtensionTabUtil::IsKillURL(const GURL& url) {
#if DCHECK_IS_ON()
  // Caller should ensure that |url| is already "fixed up" by
  // url_formatter::FixupURL, which (among many other things) takes care
  // of rewriting about:kill into chrome://kill/.
  if (url.SchemeIs(url::kAboutScheme)) {
    DCHECK(url.IsAboutBlank() || url.IsAboutSrcdoc());
  }
#endif

  // Disallow common renderer debug URLs.
  // Note: this would also disallow JavaScript URLs, but we already explicitly
  // check for those before calling into here from PrepareURLForNavigation.
  if (blink::IsRendererDebugURL(url)) {
    return true;
  }

  if (!url.SchemeIs(content::kChromeUIScheme)) {
    return false;
  }

  // Also disallow a few more hosts which are not covered by the check above.
  constexpr auto kKillHosts = base::MakeFixedFlatSet<std::string_view>({
      content::kChromeUIBrowserCrashHost,
      content::kChromeUIMemoryExhaustHost,
  });

  return kKillHosts.contains(url.host());
}

base::expected<GURL, std::string> ExtensionTabUtil::PrepareURLForNavigation(
    const std::string& url_string,
    const extensions::Extension* extension,
    content::BrowserContext* browser_context) {
  GURL url =
      ExtensionTabUtil::ResolvePossiblyRelativeURL(url_string, extension);

  // Ideally, the URL would only be "fixed" for user input (e.g. for URLs
  // entered into the Omnibox), but some extensions rely on the legacy behavior
  // where all navigations were subject to the "fixing".  See also
  // https://crbug.com/1145381.
  url = url_formatter::FixupURL(url.spec(), "" /* = desired_tld */);

  // Reject invalid URLs.
  if (!url.is_valid()) {
    return base::unexpected(extensions::ErrorUtils::FormatErrorMessage(
        tabs_constants::kInvalidUrlError, url_string));
  }

  // Don't let the extension use JavaScript URLs in API triggered navigations.
  if (url.SchemeIs(url::kJavaScriptScheme)) {
    return base::unexpected(
        tabs_constants::kJavaScriptUrlsNotAllowedInExtensionNavigations);
  }

  // Don't let the extension crash the browser or renderers.
  if (ExtensionTabUtil::IsKillURL(url)) {
    return base::unexpected(tabs_constants::kNoCrashBrowserError);
  }

  // Don't let the extension navigate directly to devtools scheme pages, unless
  // they have applicable permissions.
  if (url.SchemeIs(content::kChromeDevToolsScheme)) {
    bool has_permission =
        extension && (extension->permissions_data()->HasAPIPermission(
                          extensions::mojom::APIPermissionID::kDevtools) ||
                      extension->permissions_data()->HasAPIPermission(
                          extensions::mojom::APIPermissionID::kDebugger));
    if (!has_permission) {
      return base::unexpected(tabs_constants::kCannotNavigateToDevtools);
    }
  }

  // Don't let the extension navigate directly to chrome-untrusted scheme pages.
  if (url.SchemeIs(content::kChromeUIUntrustedScheme)) {
    return base::unexpected(tabs_constants::kCannotNavigateToChromeUntrusted);
  }

  // Don't let the extension navigate directly to file scheme pages, unless
  // they have file access. `extension` can be null if the call is made from
  // non-extension contexts (e.g. WebUI pages). In that case, we allow the
  // navigation as such contexts are trusted and do not have a concept of file
  // access.
  if (extension && IsFileUrl(url) &&
      // PDF viewer extension can navigate to file URLs.
      extension->id() != extension_misc::kPdfExtensionId &&
      !extensions::util::AllowFileAccess(extension->id(), browser_context)) {
    return base::unexpected(
        tabs_constants::kFileUrlsNotAllowedInExtensionNavigations);
  }

  return url;
}

// static
tabs::TabStatus ExtensionTabUtil::GetLoadingStatus(
    content::WebContents* contents) {
  if (contents->IsLoading()) {
    return tabs::TabStatus::kLoading;
  }

  // Anything that isn't backed by a process is considered unloaded.
  if (!HasValidMainFrameProcess(contents)) {
    return tabs::TabStatus::kUnloaded;
  }

  // Otherwise its considered loaded.
  return tabs::TabStatus::kComplete;
}

void ExtensionTabUtil::DispatchTabsOnCreated(content::BrowserContext* context,
                                             uint64_t tab_id) {
  content::WebContents* web_contents =
      NevaExtensionsServiceFactory::GetService(context)
          ->GetTabHelper()
          ->GetWebContentsFromId(tab_id);

  base::ListValue on_created_args;
  tabs::Tab tab_object;
  tab_object.id = tab_id;
  tab_object.window_id = tab_id;
  tab_object.active =
      (web_contents->GetVisibility() != content::Visibility::HIDDEN);
  tab_object.selected = tab_object.active;
  tab_object.url = web_contents->GetLastCommittedURL().spec();
  tab_object.title = base::UTF16ToUTF8(web_contents->GetTitle());
  tab_object.group_id = -1;
  tab_object.incognito = context->IsOffTheRecord();
  tab_object.highlighted = tab_object.active;
  on_created_args.Append(tab_object.ToValue());

  const std::string event_name = tabs::OnCreated::kEventName;
  if (extensions::EventRouter::Get(context)->HasEventListener(event_name)) {
    auto event = std::make_unique<extensions::Event>(
        extensions::events::TABS_ON_CREATED, event_name,
        std::move(on_created_args), context);
    extensions::EventRouter::Get(context)->BroadcastEvent(std::move(event));
  }
}

void ExtensionTabUtil::DispatchTabsOnActivated(content::BrowserContext* context,
                                               uint64_t tab_id) {
  VLOG(1) << __func__ << " tab_id: " << tab_id << " / context: " << context;
  if (!context) {
    return;
  }

  base::ListValue on_activated_args;
  base::DictValue details;
  details.Set(tabs_constants::kTabIdKey, static_cast<int>(tab_id));
  details.Set(tabs_constants::kWindowIdKey, static_cast<int>(tab_id));
  on_activated_args.Append(std::move(details));

  const std::string event_name = tabs::OnActivated::kEventName;
  if (extensions::EventRouter::Get(context)->HasEventListener(event_name)) {
    auto event = std::make_unique<extensions::Event>(
        extensions::events::TABS_ON_ACTIVATED, event_name,
        std::move(on_activated_args), context);
    extensions::EventRouter::Get(context)->BroadcastEvent(std::move(event));
  }
}

void ExtensionTabUtil::DispatchTabsOnUpdated(content::BrowserContext* context,
                                             uint64_t tab_id,
                                             const std::string& change_info) {
  VLOG(1) << __func__ << " tab_id: " << tab_id << " / context: " << context;
  if (!context) {
    return;
  }

  base::DictValue details;
  std::optional<base::DictValue> dict = base::JSONReader::ReadDict(
      change_info, base::JSON_PARSE_CHROMIUM_EXTENSIONS);
  if (!dict) {
    LOG(ERROR) << __func__ << " parsing change_info JSON has failed.";
    return;
  }
  const std::string* status = dict->FindString(tabs_constants::kStatusKey);
  if (status && *status == tabs_constants::kStatusLoading) {
    details.Set(tabs_constants::kStatusKey, tabs_constants::kStatusLoading);
  } else {
    details.Set(tabs_constants::kStatusKey, tabs_constants::kStatusComplete);
  }

  content::WebContents* web_contents =
      NevaExtensionsServiceFactory::GetService(context)
          ->GetTabHelper()
          ->GetWebContentsFromId(tab_id);
  tabs::Tab tab_object;
  tab_object.id = tab_id;
  tab_object.window_id = tab_id;
  tab_object.active =
      (web_contents->GetVisibility() != content::Visibility::HIDDEN);
  tab_object.selected = tab_object.active;
  tab_object.url = web_contents->GetLastCommittedURL().spec();
  tab_object.title = base::UTF16ToUTF8(web_contents->GetTitle());
  tab_object.group_id = -1;
  tab_object.incognito = context->IsOffTheRecord();
  tab_object.highlighted = tab_object.active;

  base::ListValue on_updated_args;
  on_updated_args.Append(static_cast<int>(tab_id));
  on_updated_args.Append(std::move(details));
  on_updated_args.Append(tab_object.ToValue());

  const std::string event_name = tabs::OnUpdated::kEventName;
  if (extensions::EventRouter::Get(context)->HasEventListener(event_name)) {
    auto event = std::make_unique<extensions::Event>(
        extensions::events::TABS_ON_UPDATED, event_name,
        std::move(on_updated_args), context);
    extensions::EventRouter::Get(context)->BroadcastEvent(std::move(event));
  }
}

void ExtensionTabUtil::DispatchTabsOnRemoved(content::BrowserContext* context,
                                             uint64_t tab_id) {
  if (!context) {
    return;
  }

  base::DictValue details;
  details.Set(tabs_constants::kWindowIdKey, static_cast<int>(tab_id));
  details.Set(tabs_constants::kIsWindowClosingKey, false);

  base::ListValue on_removed_args;
  on_removed_args.Append(static_cast<int>(tab_id));
  on_removed_args.Append(std::move(details));

  const std::string event_name = tabs::OnRemoved::kEventName;
  if (extensions::EventRouter::Get(context)->HasEventListener(event_name)) {
    auto event = std::make_unique<extensions::Event>(
        extensions::events::TABS_ON_REMOVED, event_name,
        std::move(on_removed_args), context);
    extensions::EventRouter::Get(context)->BroadcastEvent(std::move(event));
  }
}

void ExtensionTabUtil::DispatchWindowsOnCreated(
    content::BrowserContext* context,
    uint64_t tab_id) {
  if (!context) {
    return;
  }

  base::ListValue args;
  args.Append(CreateWindowValueForExtension(tab_id, true));

  const std::string event_name = windows::OnCreated::kEventName;
  if (extensions::EventRouter::Get(context)->HasEventListener(event_name)) {
    auto event = std::make_unique<extensions::Event>(
        extensions::events::WINDOWS_ON_CREATED, event_name, std::move(args),
        context);
    extensions::EventRouter::Get(context)->BroadcastEvent(std::move(event));
  }
}

void ExtensionTabUtil::DispatchWindowsOnFocusChanged(
    content::BrowserContext* context,
    uint64_t tab_id) {
  if (!context) {
    return;
  }

  base::ListValue args;
  args.Append(static_cast<int>(tab_id));

  const std::string event_name = windows::OnFocusChanged::kEventName;
  if (extensions::EventRouter::Get(context)->HasEventListener(event_name)) {
    auto event = std::make_unique<extensions::Event>(
        extensions::events::WINDOWS_ON_FOCUS_CHANGED, event_name,
        std::move(args), context);
    extensions::EventRouter::Get(context)->BroadcastEvent(std::move(event));
  }
}

void ExtensionTabUtil::DispatchWindowsOnRemoved(
    content::BrowserContext* context,
    uint64_t tab_id) {
  if (!context) {
    return;
  }

  base::ListValue args;
  args.Append(static_cast<int>(tab_id));

  const std::string event_name = windows::OnRemoved::kEventName;
  if (extensions::EventRouter::Get(context)->HasEventListener(event_name)) {
    auto event = std::make_unique<extensions::Event>(
        extensions::events::WINDOWS_ON_REMOVED, event_name, std::move(args),
        context);
    extensions::EventRouter::Get(context)->BroadcastEvent(std::move(event));
  }
}

}  // namespace neva
