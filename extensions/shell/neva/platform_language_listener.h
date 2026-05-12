// Copyright 2019 LG Electronics, Inc.
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

#ifndef EXTENSIONS_SHELL_NEVA_PLATFORM_LANGUAGE_LISTENER_H_
#define EXTENSIONS_SHELL_NEVA_PLATFORM_LANGUAGE_LISTENER_H_

#include "base/memory/weak_ptr.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_user_data.h"

namespace pal {
class LanguageTrackerDelegate;
}

class PlatformLanguageListener
    : public content::WebContentsUserData<PlatformLanguageListener>,
      public content::WebContentsObserver {
 public:
  friend class content::WebContentsUserData<PlatformLanguageListener>;

  explicit PlatformLanguageListener(content::WebContents* web_contents);
  ~PlatformLanguageListener() override;
  void OnLanguageChanged(const std::string& language_string);

 private:
  base::WeakPtrFactory<PlatformLanguageListener> weak_factory_;
  std::unique_ptr<pal::LanguageTrackerDelegate> delegate_;
  WEB_CONTENTS_USER_DATA_KEY_DECL();
};

#endif  // EXTENSIONS_SHELL_NEVA_PLATFORM_LANGUAGE_LISTENER_H_
