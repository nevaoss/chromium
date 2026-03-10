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
// //chrome/browser/extensions/api/cookies/cookies_api.cc

// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Implements the Chrome Extensions Cookies API.

#include "neva/extensions/browser/api/cookies/cookies_api.h"

#include "base/lazy_instance.h"
#include "base/values.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/storage_partition.h"
#include "extensions/common/error_utils.h"
#include "extensions/common/permissions/permissions_data.h"
#include "neva/extensions/browser/api/cookies/cookies_helpers.h"
#include "services/network/public/mojom/cookie_manager.mojom.h"

namespace neva {

namespace {

// Errors
constexpr char kCookieManagerGetFailedError[] = "Failed to get Cookie Manager.";
constexpr char kCookieSetFailedError[] =
    "Failed to parse or set cookie named \"*\".";
constexpr char kInvalidUrlError[] = "Invalid url: \"*\".";
constexpr char kNoHostPermissionsError[] =
    "No host permissions for cookies at url: \"*\".";

bool ParseUrl(const extensions::Extension* extension,
              const std::string& url_string,
              GURL* url,
              bool check_host_permissions,
              std::string* error) {
  *url = GURL(url_string);
  if (!url->is_valid()) {
    *error = extensions::ErrorUtils::FormatErrorMessage(kInvalidUrlError,
                                                        url_string);
    return false;
  }
  // Check against host permissions if needed.
  if (check_host_permissions &&
      !extension->permissions_data()->HasHostPermission(*url)) {
    *error = extensions::ErrorUtils::FormatErrorMessage(kNoHostPermissionsError,
                                                        url->spec());
    return false;
  }
  return true;
}

}  // namespace

CookiesGetFunction::CookiesGetFunction() = default;
CookiesGetFunction::~CookiesGetFunction() = default;

ExtensionFunction::ResponseAction CookiesGetFunction::Run() {
  parsed_args_ = extensions::api::cookies::Get::Params::Create(args());
  EXTENSION_FUNCTION_VALIDATE(parsed_args_);

  // Read/validate input parameters.
  std::string error;
  if (!ParseUrl(extension(), parsed_args_->details.url, &url_, true, &error)) {
    return RespondNow(Error(std::move(error)));
  }

  network::mojom::CookieManager* cookie_manager =
      browser_context()
          ->GetDefaultStoragePartition()
          ->GetCookieManagerForBrowserProcess();
  if (!cookie_manager) {
    return RespondNow(Error(kCookieManagerGetFailedError));
  }

  DCHECK(!url_.is_empty() && url_.is_valid());
  cookies_helpers::GetCookieListFromManager(
      cookie_manager, url_,
      net::CookiePartitionKeyCollection(),
      base::BindOnce(&CookiesGetFunction::GetCookieListCallback, this));
  return RespondLater();
}

void CookiesGetFunction::GetCookieListCallback(
    const net::CookieAccessResultList& cookie_list,
    const net::CookieAccessResultList& excluded_cookies) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  for (const net::CookieWithAccessResult& cookie_with_access_result :
       cookie_list) {
    if (!cookies_helpers::
            CanonicalCookiePartitionKeyMatchesApiCookiePartitionKey(
                parsed_args_->details.partition_key,
                cookie_with_access_result.cookie.PartitionKey())) {
      continue;
    }

    // Return the first matching cookie. Relies on the fact that the
    // CookieManager interface returns them in canonical order (longest path,
    // then earliest creation time).
    if (cookie_with_access_result.cookie.Name() == parsed_args_->details.name) {
      extensions::api::cookies::Cookie api_cookie =
          cookies_helpers::CreateCookie(cookie_with_access_result.cookie);
      Respond(ArgumentList(
          extensions::api::cookies::Get::Results::Create(api_cookie)));
      return;
    }
  }

  // The cookie doesn't exist; return null.
  Respond(WithArguments(base::Value()));
}

CookiesGetAllFunction::CookiesGetAllFunction() {}

CookiesGetAllFunction::~CookiesGetAllFunction() {}

void CookiesGetAllFunction::GetAllCookiesCallback(
    const net::CookieList& cookie_list) {
  auto domain = parsed_args_->details.domain;

  std::vector<extensions::api::cookies::Cookie> cookies;
  cookies.reserve(cookie_list.size());

  for (const auto& cookie : cookie_list) {
    if (!domain || cookie.Domain() == domain) {
      cookies.push_back(cookies_helpers::CreateCookie(cookie));
    }
  }

  base::ListValue result;
  for (const auto& cookie : cookies) {
    result.Append(cookie.ToValue());
  }

  Respond(WithArguments(std::move(result)));
}

ExtensionFunction::ResponseAction CookiesGetAllFunction::Run() {
  parsed_args_ = extensions::api::cookies::GetAll::Params::Create(args());
  EXTENSION_FUNCTION_VALIDATE(parsed_args_);

  network::mojom::CookieManager* cookie_manager =
      browser_context()
          ->GetDefaultStoragePartition()
          ->GetCookieManagerForBrowserProcess();
  if (!cookie_manager) {
    return RespondNow(Error(kCookieManagerGetFailedError));
  }

  cookies_helpers::GetAllCookiesFromManager(
      cookie_manager,
      base::BindOnce(&CookiesGetAllFunction::GetAllCookiesCallback, this));
  return RespondLater();
}

CookiesSetFunction::CookiesSetFunction()
    : state_(NO_RESPONSE), success_(false) {}

CookiesSetFunction::~CookiesSetFunction() {}

ExtensionFunction::ResponseAction CookiesSetFunction::Run() {
  parsed_args_ = extensions::api::cookies::Set::Params::Create(args());
  EXTENSION_FUNCTION_VALIDATE(parsed_args_);

  // Read/validate input parameters.
  std::string error;
  if (!ParseUrl(extension(), parsed_args_->details.url, &url_, true, &error)) {
    return RespondNow(Error(std::move(error)));
  }

  network::mojom::CookieManager* cookie_manager =
      browser_context()
          ->GetDefaultStoragePartition()
          ->GetCookieManagerForBrowserProcess();
  if (!cookie_manager) {
    return RespondNow(Error(kCookieManagerGetFailedError));
  }

  base::Time expiration_time;
  if (parsed_args_->details.expiration_date) {
    // Time::FromSecondsSinceUnixEpoch converts double time 0 to empty Time
    // object. So we need to do special handling here.
    expiration_time = (*parsed_args_->details.expiration_date == 0)
                          ? base::Time::UnixEpoch()
                          : base::Time::FromSecondsSinceUnixEpoch(
                                *parsed_args_->details.expiration_date);
  }

  net::CookieSameSite same_site = net::CookieSameSite::UNSPECIFIED;
  switch (parsed_args_->details.same_site) {
    case extensions::api::cookies::SameSiteStatus::kNoRestriction:
      same_site = net::CookieSameSite::NO_RESTRICTION;
      break;
    case extensions::api::cookies::SameSiteStatus::kLax:
      same_site = net::CookieSameSite::LAX_MODE;
      break;
    case extensions::api::cookies::SameSiteStatus::kStrict:
      same_site = net::CookieSameSite::STRICT_MODE;
      break;
    // This is the case if the optional sameSite property is given as
    // "unspecified":
    case extensions::api::cookies::SameSiteStatus::kUnspecified:
    // This is the case if the optional sameSite property is left out:
    case extensions::api::cookies::SameSiteStatus::kNone:
      same_site = net::CookieSameSite::UNSPECIFIED;
      break;
  }

  std::unique_ptr<net::CanonicalCookie> cc(
      net::CanonicalCookie::CreateSanitizedCookie(
          url_,                                                  //
          parsed_args_->details.name.value_or(std::string()),    //
          parsed_args_->details.value.value_or(std::string()),   //
          parsed_args_->details.domain.value_or(std::string()),  //
          parsed_args_->details.path.value_or(std::string()),    //
          /*creation_time=*/base::Time(),                        //
          expiration_time,                                       //
          /*last_access_time=*/base::Time(),                     //
          parsed_args_->details.secure.value_or(false),          //
          parsed_args_->details.http_only.value_or(false),       //
          same_site,                                             //
          net::COOKIE_PRIORITY_DEFAULT,                          //
          /*partition_key=*/std::nullopt,                        //
          /*status=*/nullptr));
  if (!cc) {
    // Return error through callbacks so that the proper error message
    // is generated.
    success_ = false;
    state_ = SET_COMPLETED;
    GetCookieListCallback(net::CookieAccessResultList(),
                          net::CookieAccessResultList());
    return AlreadyResponded();
  }

  // Dispatch the setter, immediately followed by the getter.  This
  // plus FIFO ordering on the cookie_manager_ pipe means that no
  // other extension function will affect the get result.
  net::CookieOptions options;
  options.set_include_httponly();
  options.set_same_site_cookie_context(
      net::CookieOptions::SameSiteCookieContext::MakeInclusive());
  DCHECK(!url_.is_empty() && url_.is_valid());
  cookie_manager->SetCanonicalCookie(
      *cc, url_, options,
      base::BindOnce(&CookiesSetFunction::SetCanonicalCookieCallback, this));
  cookies_helpers::GetCookieListFromManager(
      cookie_manager, url_,
      net::CookiePartitionKeyCollection(),
      base::BindOnce(&CookiesSetFunction::GetCookieListCallback, this));

  // Will finish asynchronously.
  return RespondLater();
}

void CookiesSetFunction::SetCanonicalCookieCallback(
    net::CookieAccessResult set_cookie_result) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  DCHECK_EQ(NO_RESPONSE, state_);
  state_ = SET_COMPLETED;
  success_ = set_cookie_result.status.IsInclude();
}

void CookiesSetFunction::GetCookieListCallback(
    const net::CookieAccessResultList& cookie_list,
    const net::CookieAccessResultList& excluded_cookies) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  DCHECK_EQ(SET_COMPLETED, state_);
  state_ = GET_COMPLETED;

  if (!success_) {
    std::string name = parsed_args_->details.name.value_or(std::string());
    Respond(Error(extensions::ErrorUtils::FormatErrorMessage(
        kCookieSetFailedError, name)));
    return;
  }

  std::optional<ResponseValue> value;
  for (const net::CookieWithAccessResult& cookie_with_access_result :
       cookie_list) {
    // Return the first matching cookie. Relies on the fact that the
    // CookieMonster returns them in canonical order (longest path, then
    // earliest creation time).

    if (!cookies_helpers::
            CanonicalCookiePartitionKeyMatchesApiCookiePartitionKey(
                parsed_args_->details.partition_key,
                cookie_with_access_result.cookie.PartitionKey())) {
      continue;
    }

    std::string name = parsed_args_->details.name.value_or(std::string());

    if (cookie_with_access_result.cookie.Name() == name) {
      extensions::api::cookies::Cookie api_cookie =
          cookies_helpers::CreateCookie(cookie_with_access_result.cookie);
      value.emplace(ArgumentList(
          extensions::api::cookies::Set::Results::Create(api_cookie)));
      break;
    }
  }

  Respond(value ? std::move(*value) : NoArguments());
}

CookiesRemoveFunction::CookiesRemoveFunction() {}

CookiesRemoveFunction::~CookiesRemoveFunction() {}

ExtensionFunction::ResponseAction CookiesRemoveFunction::Run() {
  parsed_args_ = extensions::api::cookies::Remove::Params::Create(args());
  EXTENSION_FUNCTION_VALIDATE(parsed_args_);

  // Read/validate input parameters.
  std::string error;
  if (!ParseUrl(extension(), parsed_args_->details.url, &url_, true, &error)) {
    return RespondNow(Error(std::move(error)));
  }

  network::mojom::CookieManager* cookie_manager =
      browser_context()
          ->GetDefaultStoragePartition()
          ->GetCookieManagerForBrowserProcess();
  if (!cookie_manager) {
    return RespondNow(Error(kCookieManagerGetFailedError));
  }

  network::mojom::CookieDeletionFilterPtr filter(
      network::mojom::CookieDeletionFilter::New());

  filter->cookie_partition_key_collection =
      net::CookiePartitionKeyCollection();
  filter->url = url_;
  filter->cookie_name = parsed_args_->details.name;
  cookie_manager->DeleteCookies(
      std::move(filter),
      base::BindOnce(&CookiesRemoveFunction::RemoveCookieCallback, this));

  // Will return asynchronously.
  return RespondLater();
}

void CookiesRemoveFunction::RemoveCookieCallback(uint32_t /* num_deleted */) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  // Build the callback result
  extensions::api::cookies::Remove::Results::Details details;
  details.name = parsed_args_->details.name;
  details.url = url_.spec();
  details.store_id = "";

  Respond(
      ArgumentList(extensions::api::cookies::Remove::Results::Create(details)));
}

CookiesAPI::CookiesAPI(content::BrowserContext* context)
    : browser_context_(context) {}

CookiesAPI::~CookiesAPI() = default;

void CookiesAPI::Shutdown() {}

static base::LazyInstance<
    extensions::BrowserContextKeyedAPIFactory<CookiesAPI>>::DestructorAtExit
    g_cookies_api_factory = LAZY_INSTANCE_INITIALIZER;

// static
extensions::BrowserContextKeyedAPIFactory<CookiesAPI>*
CookiesAPI::GetFactoryInstance() {
  return g_cookies_api_factory.Pointer();
}

}  // namespace neva
