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
// chrome/browser/tab_contents/web_contents_collection.h
//
// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NEVA_APP_RUNTIME_BROWSER_WEB_CONTENTS_COLLECTION_H_
#define NEVA_APP_RUNTIME_BROWSER_WEB_CONTENTS_COLLECTION_H_

#include <memory>

#include "base/containers/flat_map.h"
#include "content/public/browser/web_contents_observer.h"

namespace content {
class WebContents;
}  // namespace content

namespace neva_app_runtime {

class WebContentsCollection {
 public:
  class Observer {
   public:
    virtual void WebContentsDestroyed(content::WebContents* web_contents) {}
    virtual void PrimaryMainFrameRenderProcessGone(
        content::WebContents* web_contents,
        base::TerminationStatus status) {}
    virtual void NavigationEntryCommitted(
        content::WebContents* web_contents,
        const content::LoadCommittedDetails& load_details) {}

   protected:
    virtual ~Observer() = default;
  };

  explicit WebContentsCollection(Observer* observer);
  ~WebContentsCollection();

  void StartObserving(content::WebContents* web_contents);
  void StopObserving(content::WebContents* web_contents);

 private:
  class ForwardingWebContentsObserver;

  void WebContentsDestroyed(content::WebContents* web_contents);

  const raw_ptr<Observer> observer_;
  base::flat_map<content::WebContents*,
                 std::unique_ptr<ForwardingWebContentsObserver>>
      web_contents_observers_;
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_BROWSER_WEB_CONTENTS_COLLECTION_H_
