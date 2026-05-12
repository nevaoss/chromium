// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/shell/browser/shell_extension_web_contents_observer.h"

#if defined(USE_NEVA_APPRUNTIME)
#include "base/command_line.h"
#include "base/json/string_escape.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/navigation_handle.h"
#include "content/public/browser/render_frame_host.h"
#include "extensions/shell/common/switches.h"
#endif

namespace extensions {

ShellExtensionWebContentsObserver::ShellExtensionWebContentsObserver(
    content::WebContents* web_contents)
    : ExtensionWebContentsObserver(web_contents),
      content::WebContentsUserData<ShellExtensionWebContentsObserver>(
          *web_contents) {}

ShellExtensionWebContentsObserver::~ShellExtensionWebContentsObserver() {
}

#if defined(USE_NEVA_APPRUNTIME)
void ShellExtensionWebContentsObserver::DidFinishNavigation(
    content::NavigationHandle* navigation_handle) {
  ExtensionWebContentsObserver::DidFinishNavigation(navigation_handle);

  if (navigation_handle->HasCommitted() &&
      navigation_handle->GetURL().SchemeIs(kExtensionScheme) &&
      base::CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kLaunchArgs)) {
    content::RenderFrameHost* render_frame_host =
        navigation_handle->GetRenderFrameHost();
    const std::string launch_args =
        base::CommandLine::ForCurrentProcess()->GetSwitchValueNative(
            switches::kLaunchArgs);
    const std::string js_line =
        std::string(" chrome.app.launchArgs=") +
        base::GetQuotedJSONString(launch_args) +
        std::string(";");
    render_frame_host->ExecuteJavaScript(base::UTF8ToUTF16(js_line),
                                         base::NullCallback());
  }
}
#endif

void ShellExtensionWebContentsObserver::CreateForWebContents(
    content::WebContents* web_contents) {
  content::WebContentsUserData<
      ShellExtensionWebContentsObserver>::CreateForWebContents(web_contents);

  // Initialize this instance if necessary.
  FromWebContents(web_contents)->Initialize();
}

WEB_CONTENTS_USER_DATA_KEY_IMPL(ShellExtensionWebContentsObserver);

}  // namespace extensions
