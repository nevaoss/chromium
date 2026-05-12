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

#include "neva/extensions/browser/neva_extensions_service_impl.h"

#include "content/public/browser/render_process_host.h"
#include "content/public/browser/web_contents.h"
#include "extensions/browser/extension_action_manager.h"
#include "extensions/browser/extension_host.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/process_manager.h"
#include "neva/extensions/browser/api/extension_action/extension_action_api.h"
#include "neva/extensions/browser/extension_tab_util.h"
#include "neva/extensions/browser/neva_extensions_service_factory.h"
#include "neva/extensions/browser/tab_helper.h"
#include "neva/logging.h"
#include "third_party/blink/public/mojom/renderer_preferences.mojom.h"
#include "ui/views/controls/webview/webview.h"

namespace neva {

NevaExtensionsServiceImpl::TabCreationRequest::TabCreationRequest() = default;

NevaExtensionsServiceImpl::TabCreationRequest::~TabCreationRequest() = default;

NevaExtensionsServiceImpl::NevaExtensionsServiceImpl(
    content::BrowserContext* browser_context)
    : browser_context_(browser_context), popup_view_id_(0) {}

NevaExtensionsServiceImpl::~NevaExtensionsServiceImpl() = default;

// static
void NevaExtensionsServiceImpl::BindForRenderer(
    int render_process_id,
    mojo::PendingReceiver<mojom::NevaExtensionsService> receiver) {
  auto* host = content::RenderProcessHost::FromID(render_process_id);
  if (!host)
    return;

  NevaExtensionsServiceImpl* extensions_service =
      NevaExtensionsServiceFactory::GetService(host->GetBrowserContext());
  extensions_service->Bind(std::move(receiver));
}

void NevaExtensionsServiceImpl::Bind(
    mojo::PendingReceiver<mojom::NevaExtensionsService> receiver) {
  receivers_.Add(this, std::move(receiver));
}

void NevaExtensionsServiceImpl::SetTabHelper(
    std::unique_ptr<TabHelper> tab_helper) {
  tab_helper_ = std::move(tab_helper);
}

TabHelper* NevaExtensionsServiceImpl::GetTabHelper() {
  return tab_helper_.get();
}

void NevaExtensionsServiceImpl::OnExtensionTabCreationRequested(
    const std::string& url,
    const std::string& type,
    TabCreatedCB callback) {
  VLOG(1) << __func__ << " url: " << url;

  int request_id = request_id_generator_.GetNext();
  tab_creation_requests_.emplace(request_id,
                                 std::make_unique<TabCreationRequest>());
  tab_creation_requests_[request_id]->url = url;
  tab_creation_requests_[request_id]->callback = std::move(callback);

  if (client_)
    RequestTabCreationToClient(request_id, type);
  else
    pending_tab_creation_requests_.emplace_back(request_id, type);
}

void NevaExtensionsServiceImpl::OnExtensionTabCloseRequested(uint64_t tab_id) {
  VLOG(1) << __func__ << " tab_id: " << tab_id;
  NEVA_DCHECK(client_);
  client_->OnExtensionTabCloseRequested(tab_id);
}

void NevaExtensionsServiceImpl::OnExtensionTabFocusRequested(uint64_t tab_id) {
  VLOG(1) << __func__ << " tab_id: " << tab_id;
  NEVA_DCHECK(client_);
  client_->OnExtensionTabFocusRequested(tab_id);
}

void NevaExtensionsServiceImpl::RequestTabCreationToClient(
    uint64_t request_id,
    const std::string& tab_type) {
  NEVA_DCHECK(client_);
  client_->OnExtensionTabCreationRequested(request_id, tab_type);
}

void NevaExtensionsServiceImpl::UpdatePreferredLanguage(
    content::WebContents* web_contents) {
  if (preferred_language_.empty()) {
    return;
  }

  auto* renderer_prefs(web_contents->GetMutableRendererPrefs());
  if (renderer_prefs) {
    // If the language was missing, add it to the beginning of accept_languages.
    // Otherwise move language to the beginning of the accept_languages string.
    std::vector<std::string_view> languages = base::SplitStringPiece(
        renderer_prefs->accept_languages, ",", base::TRIM_WHITESPACE,
        base::SPLIT_WANT_NONEMPTY);

    if (!languages.empty() &&
        languages.front().compare(preferred_language_) == 0) {
      return;
    }

    std::vector<std::string> updated_languages;
    updated_languages.push_back(preferred_language_);
    for (const std::string_view& lang : languages) {
      if (lang.compare(updated_languages.front()) != 0) {
        updated_languages.push_back(std::string(lang));
      }
    }

    renderer_prefs->accept_languages = base::JoinString(updated_languages, ",");
    web_contents->SyncRendererPrefs();
  }
}

void NevaExtensionsServiceImpl::OnExtensionTabCreated(uint64_t tab_id) {
  content::BrowserContext* context =
      tab_helper_->GetWebContentsFromId(tab_id)->GetBrowserContext();
  VLOG(1) << __func__ << " tab_id: " << tab_id << " / context: " << context;

  if (context) {
    tab_id_to_browser_context_[tab_id] = context;

    // TODO(neva): Currently Neva browser cannot distinguish the window and the
    // tab so we should support to recognize them in the future.
    ExtensionTabUtil::DispatchTabsOnCreated(context, tab_id);
    ExtensionTabUtil::DispatchWindowsOnCreated(context, tab_id);

    for (auto* observer : tab_event_observers_) {
      observer->OnExtensionTabCreated(
          tab_helper_->GetWebContentsFromId(tab_id));
    }
  }
}

void NevaExtensionsServiceImpl::OnExtensionTabCreatedWithRequestId(
    uint64_t request_id,
    uint64_t tab_id) {
  VLOG(1) << __func__ << " request_id: " << request_id
          << " / tab_id: " << tab_id;

  content::WebContents* web_contents =
      tab_helper_->GetWebContentsFromId(tab_id);
  web_contents->GetController().LoadURL(
      GURL(tab_creation_requests_[request_id]->url), content::Referrer(),
      ui::PAGE_TRANSITION_LINK, std::string());
  std::move(tab_creation_requests_[request_id]->callback).Run(tab_id);
  tab_creation_requests_.erase(request_id);
}

void NevaExtensionsServiceImpl::OnExtensionTabClosed(uint64_t tab_id) {
  auto it = tab_id_to_browser_context_.find(tab_id);
  if (it == tab_id_to_browser_context_.end()) {
    LOG(ERROR) << __func__ << ": tab_id(" << tab_id << ") not found.";
    return;
  }
  content::BrowserContext* context = it->second;
  tab_id_to_browser_context_.erase(it);
  VLOG(1) << __func__ << " tab_id: " << tab_id << " / context: " << context;

  if (context) {
    // TODO(neva): Currently Neva browser cannot distinguish the window and the
    // tab so we should support to recognize them in the future.
    ExtensionTabUtil::DispatchTabsOnRemoved(context, tab_id);
    ExtensionTabUtil::DispatchWindowsOnRemoved(context, tab_id);
  }
}

void NevaExtensionsServiceImpl::OnExtensionTabActivated(uint64_t tab_id) {
  auto it = tab_id_to_browser_context_.find(tab_id);
  if (it == tab_id_to_browser_context_.end()) {
    LOG(ERROR) << __func__ << ": tab_id(" << tab_id << ") not found.";
    return;
  }

  // TODO(neva): Currently Neva browser cannot distinguish the window and the
  // tab so we should support to recognize them in the future.
  ExtensionTabUtil::DispatchTabsOnActivated(it->second, tab_id);
  ExtensionTabUtil::DispatchWindowsOnFocusChanged(it->second, tab_id);
}

void NevaExtensionsServiceImpl::OnExtensionTabUpdated(
    uint64_t tab_id,
    const std::string& change_info) {
  auto it = tab_id_to_browser_context_.find(tab_id);
  if (it == tab_id_to_browser_context_.end()) {
    LOG(ERROR) << __func__ << ": tab_id(" << tab_id << ") not found.";
    return;
  }
  ExtensionTabUtil::DispatchTabsOnUpdated(it->second, tab_id, change_info);
}

void NevaExtensionsServiceImpl::BindClient(
    mojo::PendingRemote<mojom::NevaExtensionsServiceClient> client) {
  VLOG(1) << __func__;

  client_.Bind(std::move(client));

  if (pending_tab_creation_requests_.size() > 0) {
    for (auto& request : pending_tab_creation_requests_) {
      RequestTabCreationToClient(request.first, request.second);
    }
    pending_tab_creation_requests_.clear();
  }
}

void NevaExtensionsServiceImpl::GetExtensionsInfo(
    GetExtensionsInfoCallback callback) {
  std::vector<base::Value> extension_infos;
  extensions::ExtensionRegistry* registry =
      extensions::ExtensionRegistry::Get(browser_context_);
  const extensions::ExtensionSet& extensions = registry->enabled_extensions();

  for (auto& extension : extensions) {
    base::Value::Dict dict;
    dict.Set("id", extension->id());
    dict.Set("name", extension->name());
    extension_infos.push_back(base::Value(std::move(dict)));
  }

  std::move(callback).Run(std::move(extension_infos));
}

void NevaExtensionsServiceImpl::OnExtensionSelected(
    uint64_t tab_id,
    const std::string& extension_id) {
  const extensions::Extension* extension =
      extensions::ExtensionRegistry::Get(browser_context_)
          ->GetExtensionById(
              extension_id,
              extensions::ExtensionRegistry::IncludeFlag::EVERYTHING);
  NEVA_DCHECK(extension);
  extensions::ExtensionAction* extension_action =
      extensions::ExtensionActionManager::Get(browser_context_)
          ->GetExtensionAction(*extension);
  if (extension_action && extension_action->HasPopup(tab_id)) {
    LOG(INFO) << __func__ << " Popup creation Requested.";
    popup_extension_id_ = extension_id;
    client_->OnExtensionPopupCreationRequested();
    return;
  }

  ExtensionActionAPI::Get(browser_context_)
      ->DispatchExtensionActionClicked(
          extension_id, tab_id, tab_helper_->GetWebContentsFromId(tab_id));
}

void NevaExtensionsServiceImpl::OnExtensionPopupViewCreated(
    uint64_t popup_view_id,
    uint64_t tab_id) {
  VLOG(1) << __func__ << " / popup_view_id: " << popup_view_id;

  std::string extension_id = popup_extension_id_;
  popup_extension_id_ = std::string();

  views::View* popup_view = tab_helper_->GetViewFromId(popup_view_id);
  // neva_app_runtime::PageView returns only views::View. So we need to cast.
  views::WebView* web_view = static_cast<views::WebView*>(popup_view);
  if (!web_view) {
    LOG(ERROR) << __func__ << " Cannot get WebView for extension popup.";
    return;
  }

  const extensions::Extension* extension =
      extensions::ExtensionRegistry::Get(browser_context_)
          ->GetExtensionById(
              extension_id,
              extensions::ExtensionRegistry::IncludeFlag::EVERYTHING);
  extensions::ExtensionAction* extension_action =
      extensions::ExtensionActionManager::Get(browser_context_)
          ->GetExtensionAction(*extension);
  if (!extension_action) {
    return;
  }

  GURL popup_url = extension_action->GetPopupUrl(tab_id);
  scoped_refptr<content::SiteInstance> site_instance =
      extensions::ProcessManager::Get(browser_context_)
          ->GetSiteInstanceForURL(popup_url);

  popup_extension_host_ = std::make_unique<NevaExtensionHost>(
      extension, site_instance.get(), popup_url,
      extensions::mojom::ViewType::kExtensionPopup);
  popup_extension_host_->CreateRendererSoon();
  web_view->SetWebContents(popup_extension_host_->host_contents());
  web_view->AddObserver(this);
  web_view->EnableSizingFromWebContents(kMinSize, kMaxSize);
  popup_view_id_ = popup_view_id;

  // The base::Unretained() below is safe because this object owns
  // `popup_extension_host_`, so the callback will never fire if `this` is
  // deleted.
  popup_extension_host_->SetCloseHandler(
      base::BindOnce(&NevaExtensionsServiceImpl::HandleCloseExtensionHost,
                     base::Unretained(this)));

  popup_extension_host_->SetSizeUpdateHandler(base::BindRepeating(
      &NevaExtensionsServiceImpl::HandleUpdateExtensionPopup,
      base::Unretained(this)));
}

void NevaExtensionsServiceImpl::OnViewIsDeleting(views::View* observed_view) {
  popup_extension_host_.reset();
}

void NevaExtensionsServiceImpl::OnViewVisibilityChanged(
    views::View* observed_view,
    views::View* starting_view) {
  if (popup_extension_host_ && !observed_view->HasFocus()) {
    popup_extension_host_.reset();
  }
}

void NevaExtensionsServiceImpl::RegisterTabEventObserver(
    TabEventObserver* observer) {
  auto it = std::ranges::find(tab_event_observers_, observer);
  if (it == tab_event_observers_.end()) {
    tab_event_observers_.push_back(observer);
  }
}

void NevaExtensionsServiceImpl::UnregisterTabEventObserver(
    TabEventObserver* observer) {
  auto it = std::ranges::find(tab_event_observers_, observer);
  if (it != tab_event_observers_.end()) {
    tab_event_observers_.erase(it);
  }
}

void NevaExtensionsServiceImpl::HandleCloseExtensionHost(
    extensions::ExtensionHost* host) {
  DCHECK_EQ(host, popup_extension_host_.get());
  client_->OnExtensionPopupCloseRequested(popup_view_id_);
}

void NevaExtensionsServiceImpl::HandleUpdateExtensionPopup(
    extensions::ExtensionHost* host,
    int width,
    int height) {
  DCHECK_EQ(host, popup_extension_host_.get());
  client_->OnExtensionPopupUpdateRequested(popup_view_id_, width, height);
}

}  // namespace neva
