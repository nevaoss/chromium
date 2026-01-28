// Copyright 2024 LG Electronics, Inc.
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

#include "neva/extensions/browser/api/web_navigation/frame_navigation_state.h"

#include "content/public/common/url_constants.h"
#include "extensions/common/constants.h"

namespace neva {

namespace {

// URL schemes for which we'll send events.
const char* const kValidSchemes[] = {
    content::kChromeUIScheme, url::kHttpScheme,       url::kHttpsScheme,
    url::kFileScheme,         url::kFtpScheme,        url::kJavaScriptScheme,
    url::kDataScheme,         url::kFileSystemScheme,
};

}  // namespace

// static
bool FrameNavigationState::allow_extension_scheme_ = false;

DOCUMENT_USER_DATA_KEY_IMPL(FrameNavigationState);

FrameNavigationState::FrameNavigationState(
    content::RenderFrameHost* render_frame_host)
    : content::DocumentUserData<FrameNavigationState>(render_frame_host) {}
FrameNavigationState::~FrameNavigationState() = default;

// static
bool FrameNavigationState::IsValidUrl(const GURL& url) {
  for (const auto* valid_scheme : kValidSchemes) {
    if (url.scheme() == valid_scheme) {
      return true;
    }
  }
  // Allow about:blank and about:srcdoc.
  if (url.IsAboutBlank() || url.IsAboutSrcdoc()) {
    return true;
  }
  return allow_extension_scheme_ &&
         url.scheme() == extensions::kExtensionScheme;
}

bool FrameNavigationState::CanSendEvents() const {
  return !error_occurred_ && IsValidUrl(url_);
}

void FrameNavigationState::StartTrackingDocumentLoad(
    const GURL& url,
    bool is_same_document,
    bool is_from_back_forward_cache,
    bool is_error_page) {
  error_occurred_ = is_error_page;
  url_ = url;
  if (!is_same_document && !is_from_back_forward_cache) {
    is_loading_ = true;
    is_parsing_ = true;
  }
}

GURL FrameNavigationState::GetUrl() const {
  return url_;
}

void FrameNavigationState::SetErrorOccurredInFrame() {
  error_occurred_ = true;
}

bool FrameNavigationState::GetErrorOccurredInFrame() const {
  return error_occurred_;
}

void FrameNavigationState::SetDocumentLoadCompleted() {
  is_loading_ = false;
}

bool FrameNavigationState::GetDocumentLoadCompleted() const {
  return !is_loading_;
}

void FrameNavigationState::SetParsingFinished() {
  is_parsing_ = false;
}

bool FrameNavigationState::GetParsingFinished() const {
  return !is_parsing_;
}

}  // namespace neva
