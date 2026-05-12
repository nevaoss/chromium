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
//
// Based on weblayer/browser/host_content_settings_map_factory.cc
//
// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "neva/app_runtime/browser/host_content_settings_map_factory.h"

#include <utility>

#include "base/feature_list.h"
#include "components/content_settings/core/browser/content_settings_pref_provider.h"
#include "components/content_settings/core/browser/host_content_settings_map.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "components/user_prefs/user_prefs.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "neva/logging.h"

namespace neva_app_runtime {

HostContentSettingsMapFactory::HostContentSettingsMapFactory()
    : RefcountedBrowserContextKeyedServiceFactory(
          "HostContentSettingsMap",
          BrowserContextDependencyManager::GetInstance()) {}

HostContentSettingsMapFactory::~HostContentSettingsMapFactory() = default;

// static
HostContentSettingsMap* HostContentSettingsMapFactory::GetForBrowserContext(
    content::BrowserContext* browser_context) {
  NEVA_DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

  return static_cast<HostContentSettingsMap*>(
      GetInstance()->GetServiceForBrowserContext(browser_context, true).get());
}

// static
HostContentSettingsMapFactory* HostContentSettingsMapFactory::GetInstance() {
  static base::NoDestructor<HostContentSettingsMapFactory> instance;
  return instance.get();
}

scoped_refptr<RefcountedKeyedService>
HostContentSettingsMapFactory::BuildServiceInstanceFor(
    content::BrowserContext* context) const {
  scoped_refptr<HostContentSettingsMap> settings_map =
      base::MakeRefCounted<HostContentSettingsMap>(
          user_prefs::UserPrefs::Get(context), context->IsOffTheRecord(), true,
          false, !context->IsOffTheRecord());

  return settings_map;
}

content::BrowserContext* HostContentSettingsMapFactory::GetBrowserContextToUse(
    content::BrowserContext* context) const {
  return context;
}

}  // namespace neva_app_runtime
