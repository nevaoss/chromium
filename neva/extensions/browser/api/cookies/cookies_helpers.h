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

// Based on
// //chrome/browser/extensions/api/cookies/cookies_helpers.h

// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Defines common functionality used by the implementation of the Chrome
// Extensions Cookies API implemented in
// neva/extensions/browser/api/cookies/cookies_api.cc.

#ifndef NEVA_EXTENSIONS_BROWSER_API_COOKIES_COOKIES_HELPERS_H_
#define NEVA_EXTENSIONS_BROWSER_API_COOKIES_COOKIES_HELPERS_H_

#include "net/cookies/cookie_partition_key.h"
#include "neva/extensions/common/api/cookies.h"
#include "services/network/public/mojom/cookie_manager.mojom.h"

namespace net {
class CanonicalCookie;
}

namespace neva {

namespace cookies_helpers {

// Constructs a new Cookie object representing a cookie as defined by the
// cookies API.
extensions::api::cookies::Cookie CreateCookie(
    const net::CanonicalCookie& cookie);

// Dispatch a request to the CookieManager for cookies associated with
// |url| and |partition_key_collection|.
void GetCookieListFromManager(
    network::mojom::CookieManager* manager,
    const GURL& url,
    const net::CookiePartitionKeyCollection& partition_key_collection,
    network::mojom::CookieManager::GetCookieListCallback callback);

// Dispatch a request to the CookieManager for all cookies.
void GetAllCookiesFromManager(
    network::mojom::CookieManager* manager,
    network::mojom::CookieManager::GetAllCookiesCallback callback);

// Returns true if the top_level_site values match or both optionals do not
// contain a value. For match to occur both partition keys must be serializable
// if they are present.
bool CanonicalCookiePartitionKeyMatchesApiCookiePartitionKey(
    const std::optional<extensions::api::cookies::CookiePartitionKey>&
        api_partition_key,
    const std::optional<net::CookiePartitionKey>& net_partition_key);

}  // namespace cookies_helpers
}  // namespace neva

#endif  // NEVA_EXTENSIONS_BROWSER_API_COOKIES_COOKIES_HELPERS_H_
