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
// //chrome/browser/extensions/api/cookies/cookies_helpers.cc

// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Implements common functionality for the Chrome Extensions Cookies API.

#include "neva/extensions/browser/api/cookies/cookies_helpers.h"

#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "net/cookies/canonical_cookie.h"
#include "net/cookies/cookie_util.h"

using extensions::api::cookies::Cookie;

namespace neva {

namespace cookies_helpers {

extensions::api::cookies::Cookie CreateCookie(
    const net::CanonicalCookie& canonical_cookie) {
  Cookie cookie;
  // A cookie is a raw byte sequence. By explicitly parsing it as UTF-8, we
  // apply error correction, so the string can be safely passed to the renderer.
  cookie.name = base::UTF16ToUTF8(base::UTF8ToUTF16(canonical_cookie.Name()));
  cookie.value = base::UTF16ToUTF8(base::UTF8ToUTF16(canonical_cookie.Value()));
  cookie.domain = canonical_cookie.Domain();
  cookie.host_only =
      net::cookie_util::DomainIsHostOnly(canonical_cookie.Domain());
  // A non-UTF8 path is invalid, so we just replace it with an empty string.
  cookie.path = base::IsStringUTF8(canonical_cookie.Path())
                    ? canonical_cookie.Path()
                    : std::string();
  cookie.secure = canonical_cookie.SecureAttribute();
  cookie.http_only = canonical_cookie.IsHttpOnly();

  switch (canonical_cookie.SameSite()) {
    case net::CookieSameSite::NO_RESTRICTION:
      cookie.same_site =
          extensions::api::cookies::SameSiteStatus::kNoRestriction;
      break;
    case net::CookieSameSite::LAX_MODE:
      cookie.same_site = extensions::api::cookies::SameSiteStatus::kLax;
      break;
    case net::CookieSameSite::STRICT_MODE:
      cookie.same_site = extensions::api::cookies::SameSiteStatus::kStrict;
      break;
    case net::CookieSameSite::UNSPECIFIED:
      cookie.same_site = extensions::api::cookies::SameSiteStatus::kUnspecified;
      break;
  }

  cookie.session = !canonical_cookie.IsPersistent();
  if (canonical_cookie.IsPersistent()) {
    double expiration_date =
        canonical_cookie.ExpiryDate().InSecondsFSinceUnixEpoch();
    if (canonical_cookie.ExpiryDate().is_max() ||
        !std::isfinite(expiration_date)) {
      expiration_date = std::numeric_limits<double>::max();
    }
    cookie.expiration_date = expiration_date;
  }

  if (canonical_cookie.PartitionKey()) {
    base::expected<net::CookiePartitionKey::SerializedCookiePartitionKey,
                   std::string>
        serialized_partition_key =
            net::CookiePartitionKey::Serialize(canonical_cookie.PartitionKey());
    CHECK(serialized_partition_key.has_value());
    cookie.partition_key = extensions::api::cookies::CookiePartitionKey();
    cookie.partition_key->top_level_site =
        serialized_partition_key->TopLevelSite();
    cookie.partition_key->has_cross_site_ancestor =
        serialized_partition_key->has_cross_site_ancestor();
  }
  return cookie;
}

void GetCookieListFromManager(
    network::mojom::CookieManager* manager,
    const GURL& url,
    const net::CookiePartitionKeyCollection& partition_key_collection,
    network::mojom::CookieManager::GetCookieListCallback callback) {
  manager->GetCookieList(url, net::CookieOptions::MakeAllInclusive(),
                         partition_key_collection, std::move(callback));
}

void GetAllCookiesFromManager(
    network::mojom::CookieManager* manager,
    network::mojom::CookieManager::GetAllCookiesCallback callback) {
  manager->GetAllCookies(std::move(callback));
}

bool CanonicalCookiePartitionKeyMatchesApiCookiePartitionKey(
    const std::optional<extensions::api::cookies::CookiePartitionKey>&
        api_partition_key,
    const std::optional<net::CookiePartitionKey>& net_partition_key) {
  if (!net_partition_key.has_value()) {
    // Check to see if the api_partition_key is also unpartitioned.
    return !api_partition_key.has_value() ||
           !api_partition_key->top_level_site.has_value() ||
           api_partition_key.value().top_level_site.value().empty();
  }

  if (api_partition_key->has_cross_site_ancestor.has_value() &&
      net_partition_key->IsThirdParty() !=
          api_partition_key->has_cross_site_ancestor.value()) {
    return false;
  }

  // If both keys are present, they both must be serializable for a match.
  if (!net_partition_key->IsSerializeable() ||
      !api_partition_key->top_level_site.has_value()) {
    return false;
  }

  base::expected<net::CookiePartitionKey::SerializedCookiePartitionKey,
                 std::string>
      net_serialized_result =
          net::CookiePartitionKey::Serialize(net_partition_key);

  if (!net_serialized_result.has_value()) {
    return false;
  }

  return net_serialized_result->TopLevelSite() ==
         api_partition_key->top_level_site.value();
}

}  // namespace cookies_helpers
}  // namespace neva
