// Copyright 2025 LG Electronics, Inc.
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
//
// Based on
// chrome/browser/tab_contents/web_contents_collection.cc
//
// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "neva/app_runtime/browser/web_contents_collection.h"

#include "base/check.h"
#include "base/memory/raw_ptr.h"
#include "content/public/browser/web_contents_observer.h"

namespace neva_app_runtime {

class WebContentsCollection::ForwardingWebContentsObserver
    : public content::WebContentsObserver {
 public:
  ForwardingWebContentsObserver(content::WebContents* web_contents,
                                WebContentsCollection::Observer* observer,
                                WebContentsCollection* collection)
      : content::WebContentsObserver(web_contents),
        observer_(observer),
        collection_(collection) {}

 private:
  // WebContentsObserver:
  void WebContentsDestroyed() override {
    collection_->WebContentsDestroyed(web_contents());
  }

  void PrimaryMainFrameRenderProcessGone(
      base::TerminationStatus status) override {
    observer_->PrimaryMainFrameRenderProcessGone(web_contents(), status);
  }

  void NavigationEntryCommitted(
      const content::LoadCommittedDetails& load_details) override {
    observer_->NavigationEntryCommitted(web_contents(), load_details);
  }

  raw_ptr<WebContentsCollection::Observer> observer_;
  raw_ptr<WebContentsCollection> collection_;
};

WebContentsCollection::WebContentsCollection(
    WebContentsCollection::Observer* observer)
    : observer_(observer) {}

WebContentsCollection::~WebContentsCollection() = default;

void WebContentsCollection::StartObserving(content::WebContents* web_contents) {
  if (web_contents_observers_.find(web_contents) !=
      web_contents_observers_.end()) {
    return;
  }

  auto emplace_result = web_contents_observers_.emplace(
      web_contents, std::make_unique<ForwardingWebContentsObserver>(
                        web_contents, observer_, this));
  DCHECK(emplace_result.second);
}

void WebContentsCollection::StopObserving(content::WebContents* web_contents) {
  web_contents_observers_.erase(web_contents);
}

void WebContentsCollection::WebContentsDestroyed(
    content::WebContents* web_contents) {
  web_contents_observers_.erase(web_contents);
  observer_->WebContentsDestroyed(web_contents);
}

}  // namespace neva_app_runtime
