// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/waap/initial_web_ui_manager.h"

#include "base/supports_user_data.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/themes/theme_service.h"
#include "chrome/browser/themes/theme_service_factory.h"
#include "chrome/browser/ui/browser_window/public/browser_window_interface.h"
#include "chrome/browser/ui/ui_features.h"
#include "chrome/browser/ui/views/toolbar/webui_toolbar_web_view.h"
#include "chrome/browser/ui/waap/initial_webui_window_metrics_manager.h"
#include "chrome/browser/ui/webui/webui_embedding_context.h"
#include "chrome/common/chrome_features.h"
#include "chrome/common/webui_url_constants.h"
#include "components/ukm/content/source_url_recorder.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/site_instance.h"
#include "content/public/browser/web_contents.h"
#include "ui/base/unowned_user_data/scoped_unowned_user_data.h"
#include "ui/color/color_provider_manager.h"
#include "ui/color/color_provider_source.h"
#include "ui/color/color_provider_utils.h"
#include "ui/native_theme/native_theme.h"

// Forward declaration to avoid circular dependency with //chrome/browser.
// This function is defined in
// //chrome/browser/page_load_metrics/page_load_metrics_initialize.cc
void InitializePageLoadMetricsForWebContents(
    content::WebContents* web_contents);

namespace {

// Lightweight `ColorProviderSource` for use during WebUI prewarming.
// During the prewarming phase, `WebContents` are created in the background
// before any `views::Widget` is instantiated. Since `WebContents` requires a
// `ColorProviderSource` to resolve theme colors, we use this class as a
// temporary placeholder that holds the calculated `ColorProviderKey` without
// relying on a full widget hierarchy.
class PrewarmColorProviderSource : public ui::ColorProviderSource,
                                   public base::SupportsUserData::Data {
 public:
  static constexpr char kUserDataKey[] = "prewarm_color_provider_source";

  explicit PrewarmColorProviderSource(ui::ColorProviderKey key) : key_(key) {}
  ~PrewarmColorProviderSource() override = default;

  // ui::ColorProviderSource:
  const ui::ColorProvider* GetColorProvider() const override {
    return ui::ColorProviderManager::Get().GetColorProviderFor(key_);
  }
  ui::RendererColorMap GetRendererColorMap(
      ui::ColorProviderKey::ColorMode color_mode,
      ui::ColorProviderKey::ForcedColors forced_colors) const override {
    auto key = key_;
    key.color_mode = color_mode;
    key.forced_colors = forced_colors;
    const ui::ColorProvider* color_provider =
        ui::ColorProviderManager::Get().GetColorProviderFor(key);
    CHECK(color_provider);
    return ui::CreateRendererColorMap(*color_provider);
  }
  ui::ColorProviderKey GetColorProviderKey() const override { return key_; }

 private:
  const ui::ColorProviderKey key_;
};

}  // namespace

DEFINE_USER_DATA(InitialWebUIManager);

InitialWebUIManager::InitialWebUIManager(BrowserWindowInterface* browser)
    : is_initial_web_ui_pending_(
          features::IsWebUIToolbarEnabled() ||
          base::FeatureList::IsEnabled(
              features::kWebUIToolbarProcessOverheadExperiment)),
      scoped_data_holder_(browser->GetUnownedUserDataHost(), *this),
      metrics_manager_(InitialWebUIWindowMetricsManager::From(browser)) {
  if ((features::IsWebUIToolbarEnabled() &&
       features::kWebUIReloadButtonPrewarmWebUI.Get()) ||
      base::FeatureList::IsEnabled(
          features::kWebUIToolbarProcessOverheadExperiment)) {
    Profile* profile = browser->GetProfile();
    scoped_refptr<content::SiteInstance> site_instance =
        content::SiteInstance::CreateForURL(
            profile, GURL(chrome::kChromeUIWebUIToolbarURL));
    toolbar_web_contents_ = content::WebContents::Create(
        content::WebContents::CreateParams(profile, site_instance));

    ConfigureToolbarWebContents(toolbar_web_contents_.get(), browser);

    const bool pre_navigate =
        features::kWebUIReloadButtonPrewarmWebUIPreNavigate.Get() ||
        base::FeatureList::IsEnabled(
            features::kWebUIToolbarProcessOverheadExperiment);
    // We only navigate here when `WebUIReloadButtonPrewarmWebUIPreNavigate` is
    // true or we are running the process creation overhead experiment.
    if (pre_navigate) {
      toolbar_web_contents_->GetController().LoadURL(
          GURL(chrome::kChromeUIWebUIToolbarURL), content::Referrer(),
          ui::PAGE_TRANSITION_AUTO_TOPLEVEL, std::string());
    } else {
      // Only create the render process here. The navigation will be done in
      // `WebUIToolbarWebView::AddedToWidget()`.
      site_instance->GetProcess()->Init();
    }
  }
}

InitialWebUIManager::~InitialWebUIManager() = default;

std::unique_ptr<content::WebContents>
InitialWebUIManager::TakeToolbarContents() {
  return std::move(toolbar_web_contents_);
}

// static
InitialWebUIManager* InitialWebUIManager::From(
    BrowserWindowInterface* browser_window_interface) {
  return Get(browser_window_interface->GetUnownedUserDataHost());
}

// static
void InitialWebUIManager::ConfigureToolbarWebContents(
    content::WebContents* web_contents,
    BrowserWindowInterface* browser) {
  // PLM has to be initialized before loading the URL.
  InitializePageLoadMetricsForWebContents(web_contents);
  // Needed for UKM PageLoad metrics.
  ukm::InitializeSourceUrlRecorderForWebContents(web_contents);

  web_contents->SetPageBaseBackgroundColor(SK_ColorTRANSPARENT);
  web_contents->SetIgnoreZoomGestures(true);

  // Set up the color provider source so that early navigation can resolve
  // theme color properties.
  Profile* profile = browser->GetProfile();
  auto* theme_service = ThemeServiceFactory::GetForProfile(profile);
  ui::ColorProviderKey key = theme_service->GetColorProviderKey(
      ui::NativeTheme::GetInstanceForNativeUi()->GetColorProviderKey(nullptr),
      profile);
  auto source = std::make_unique<PrewarmColorProviderSource>(key);
  web_contents->SetColorProviderSource(source.get());
  web_contents->SetUserData(PrewarmColorProviderSource::kUserDataKey,
                            std::move(source));

  // Ensure the browser window interface is associated with the WebContents
  // before the WebUI acts on it.
  webui::SetBrowserWindowInterface(web_contents, browser);
}

bool InitialWebUIManager::RequestDeferShow(base::OnceClosure unsafe_callback) {
  if (metrics_manager_) {
    metrics_manager_->OnBrowserWindowShowRequested(base::TimeTicks::Now());
  }
  if (!base::FeatureList::IsEnabled(features::kWebUIReloadButton) ||
      !features::kWebUIReloadButtonDeferBrowserViewShow.Get()) {
    // Do not defer if the experiment is disabled or the param is false.
    return false;
  }
  if (is_initial_web_ui_pending_) {
    is_show_pending_ = true;
    if (unsafe_callback) {
      web_ui_ready_callbacks_.AddUnsafe(std::move(unsafe_callback));
    }
    return true;
  }
  return false;
}

bool InitialWebUIManager::IsShowPending() const {
  return is_show_pending_;
}

void InitialWebUIManager::OnWebUIToolbarLoaded() {
  is_initial_web_ui_pending_ = false;
  is_show_pending_ = false;
  web_ui_ready_callbacks_.Notify();
}
