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
// //chrome/browser/extensions/api/cookies/cookies_api.h

// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Defines the Chrome Extensions Cookies API functions for accessing internet
// cookies, as specified in the extension API JSON.

#ifndef NEVA_EXTENSIONS_BROWSER_API_COOKIES_COOKIES_API_H_
#define NEVA_EXTENSIONS_BROWSER_API_COOKIES_COOKIES_API_H_

#include "base/memory/raw_ptr.h"
#include "extensions/browser/browser_context_keyed_api_factory.h"
#include "extensions/browser/extension_function.h"
#include "net/cookies/canonical_cookie.h"
#include "neva/extensions/common/api/cookies.h"

namespace neva {

// Implements the cookies.get() extension function.
class CookiesGetFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("cookies.get", COOKIES_GET)

  CookiesGetFunction();

 protected:
  ~CookiesGetFunction() override;

  // ExtensionFunction:
  ResponseAction Run() override;

 private:
  void GetCookieListCallback(
      const net::CookieAccessResultList& cookie_list,
      const net::CookieAccessResultList& excluded_cookies);

  GURL url_;
  std::optional<extensions::api::cookies::Get::Params> parsed_args_;
};

// Implements the cookies.getAll() extension function.
class CookiesGetAllFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("cookies.getAll", COOKIES_GETALL)

  CookiesGetAllFunction();

 protected:
  ~CookiesGetAllFunction() override;

  // ExtensionFunction:
  ResponseAction Run() override;

 private:
  void GetAllCookiesCallback(const net::CookieList& cookie_list);

  std::optional<extensions::api::cookies::GetAll::Params> parsed_args_;
};

// Implements the cookies.set() extension function.
class CookiesSetFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("cookies.set", COOKIES_SET)

  CookiesSetFunction();

 protected:
  ~CookiesSetFunction() override;
  ResponseAction Run() override;

 private:
  void SetCanonicalCookieCallback(net::CookieAccessResult set_cookie_result);
  void GetCookieListCallback(
      const net::CookieAccessResultList& cookie_list,
      const net::CookieAccessResultList& excluded_cookies);

  enum { NO_RESPONSE, SET_COMPLETED, GET_COMPLETED } state_;
  GURL url_;
  bool success_;
  std::optional<extensions::api::cookies::Set::Params> parsed_args_;
};

// Implements the cookies.remove() extension function.
class CookiesRemoveFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("cookies.remove", COOKIES_REMOVE)

  CookiesRemoveFunction();

 protected:
  ~CookiesRemoveFunction() override;

  // ExtensionFunction:
  ResponseAction Run() override;

 private:
  void RemoveCookieCallback(uint32_t /* num_deleted */);

  GURL url_;
  std::optional<extensions::api::cookies::Remove::Params> parsed_args_;
};

class CookiesAPI : public extensions::BrowserContextKeyedAPI {
 public:
  explicit CookiesAPI(content::BrowserContext* context);

  CookiesAPI(const CookiesAPI&) = delete;
  CookiesAPI& operator=(const CookiesAPI&) = delete;

  ~CookiesAPI() override;

  // KeyedService implementation.
  void Shutdown() override;

  // BrowserContextKeyedAPI implementation.
  static extensions::BrowserContextKeyedAPIFactory<CookiesAPI>*
  GetFactoryInstance();

 private:
  friend class extensions::BrowserContextKeyedAPIFactory<CookiesAPI>;

  raw_ptr<content::BrowserContext> browser_context_;

  // BrowserContextKeyedAPI implementation.
  static const char* service_name() { return "CookiesAPI"; }
};

}  // namespace neva

#endif  // NEVA_EXTENSIONS_BROWSER_API_COOKIES_COOKIES_API_H_
