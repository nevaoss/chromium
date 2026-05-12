// Copyright 2018 LG Electronics, Inc.
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

#include "neva/app_runtime/renderer/app_runtime_localized_error.h"

#include "base/strings/string_number_conversions.h"
#include "net/base/net_errors.h"
#include "neva/app_runtime/grit/app_runtime_network_error_strings.h"
#include "ui/base/l10n/l10n_util.h"

namespace neva_app_runtime {
namespace {

struct AppRuntimeLocalizedErrorMap {
  int error_code;
  unsigned int title_resource_id;
  unsigned int details_resource_id;
  // Detailed summary used when the error is in the main frame.
  unsigned int error_guide_resource_id;
  // Short one sentence description shown on mouse over when the error is in
  // a frame.
  unsigned int error_info_resource_id;
};

//Currently we are handling below error cases
  // '-6'  : FILE_NOT_FOUND
  // '-7'  : TIMED_OUT
  // '-15' : SOCKET_NOT_CONNECTED
  // '-21' : NETWORK_CHANGED
  // '-100': CONNECTION_CLOSED
  // '-101': CONNECTION_RESET
  // '-102': CONNECTION_REFUSED
  // '-104': CONNECTION_FAILED
  // '-105': NAME_NOT_RESOLVED
  // '-106': INTERNET_DISCONNECTED
  // '-109': ADDRESS_UNREACHABLE
  // '-118': CONNECTION_TIMED_OUT
  // '-137': NAME_RESOLUTION_FAILED
  // '-200': CERT_COMMON_NAME_INVALID
  // '-201': CERT_DATE_INVALID
  // '-202': CERT_AUTHORITY_INVALID
  // '-324': EMPTY_RESPONSE
  // '-501': INSECURE_RESPONSE

const AppRuntimeLocalizedErrorMap net_error_options[] = {
    {
        net::ERR_FILE_NOT_FOUND,            // -6
        IDS_NET_ERROR_CHECK_NETWORK_CONNECTION,
        IDS_NET_ERROR_FILE_OR_DIRECTORY_NOT_FOUND,
        IDS_NET_ERROR_CONTACT_CONTENT_PROVIDER, IDS_NET_ERROR_EMPTY_STRING,
    },
    {
        net::ERR_TIMED_OUT,                 // -7
        IDS_NET_ERROR_CHECK_NETWORK_CONNECTION,
        IDS_NET_ERROR_OPERATION_TIMED_OUT,
        IDS_NET_ERROR_CHECK_NETWORK_STATUS_AND_TRY, IDS_NET_ERROR_EMPTY_STRING,
    },
    {
        net::ERR_SOCKET_NOT_CONNECTED,      // -15
        IDS_NET_ERROR_CHECK_NETWORK_CONNECTION, IDS_NET_ERROR_NETWORK_UNSTABLE,
        IDS_NET_ERROR_CHECK_NETWORK_STATUS_AND_TRY, IDS_NET_ERROR_EMPTY_STRING,
    },
    {
        net::ERR_NETWORK_CHANGED,           // -21
        IDS_NET_ERROR_CHECK_NETWORK_CONNECTION, IDS_NET_ERROR_NETWORK_UNSTABLE,
        IDS_NET_ERROR_CHECK_NETWORK_STATUS_AND_TRY, IDS_NET_ERROR_EMPTY_STRING,
    },
    {
        net::ERR_CONNECTION_CLOSED,         // -100
        IDS_NET_ERROR_CHECK_NETWORK_CONNECTION, IDS_NET_ERROR_TEMPORARY_ISSUE,
        IDS_NET_ERROR_TRY_AGAIN_LATER, IDS_NET_ERROR_TEMPORARY_ISSUE,
    },
    {
        net::ERR_CONNECTION_RESET,          // -101
        IDS_NET_ERROR_CHECK_NETWORK_CONNECTION, IDS_NET_ERROR_NETWORK_UNSTABLE,
        IDS_NET_ERROR_CHECK_NETWORK_STATUS_AND_TRY, IDS_NET_ERROR_EMPTY_STRING,
    },
    {
        net::ERR_CONNECTION_REFUSED,        // -102
        IDS_NET_ERROR_CHECK_NETWORK_CONNECTION,
        IDS_NET_ERROR_CONNECTION_TO_SERVER_UNSTABLE,
        IDS_NET_ERROR_CHECK_NETWORK_STATUS_AND_TRY, IDS_NET_ERROR_EMPTY_STRING,
    },
    {
        net::ERR_CONNECTION_FAILED,         // -104
        IDS_NET_ERROR_CHECK_NETWORK_CONNECTION, IDS_NET_ERROR_TEMPORARY_ISSUE,
        IDS_NET_ERROR_TRY_AGAIN_LATER, IDS_NET_ERROR_TEMPORARY_ISSUE,
    },
    {
        net::ERR_NAME_NOT_RESOLVED,         // -105
        IDS_NET_ERROR_CHECK_NETWORK_CONNECTION,
        IDS_NET_ERROR_ADDRESS_CANNOT_BE_FOUND,
        IDS_NET_ERROR_CHECK_NETWORK_CONNECTION_STATUS,
        IDS_NET_ERROR_EMPTY_STRING,
    },
    {
        net::ERR_INTERNET_DISCONNECTED,     // -106
        IDS_NET_ERROR_CHECK_NETWORK_CONNECTION,
        IDS_NET_ERROR_NETWORK_NOT_CONNECTED,
        IDS_NET_ERROR_CHECK_NETWORK_CONNECTION_STATUS,
        IDS_NET_ERROR_EMPTY_STRING,
    },
    {
        net::ERR_ADDRESS_UNREACHABLE,       // -109
        IDS_NET_ERROR_CHECK_NETWORK_CONNECTION,
        IDS_NET_ERROR_ADDRESS_CANNOT_BE_FOUND,
        IDS_NET_ERROR_CONTACT_CONTENT_PROVIDER, IDS_NET_ERROR_EMPTY_STRING,
    },
    {
        net::ERR_CONNECTION_TIMED_OUT,      // -118
        IDS_NET_ERROR_CHECK_NETWORK_CONNECTION,
        IDS_NET_ERROR_CONNECTION_ATTEMP_TIMED_OUT,
        IDS_NET_ERROR_CHECK_NETWORK_STATUS_AND_TRY, IDS_NET_ERROR_EMPTY_STRING,
    },
    {
        net::ERR_NAME_RESOLUTION_FAILED,    // -137
        IDS_NET_ERROR_CHECK_NETWORK_CONNECTION,
        IDS_NET_ERROR_ADDRESS_CANNOT_BE_FOUND,
        IDS_NET_ERROR_CHECK_NETWORK_CONNECTION_STATUS, IDS_NET_ERROR_HOSTNAME,
    },
    {
        net::ERR_CERT_COMMON_NAME_INVALID,  // -200
        IDS_NET_ERROR_UNABLE_TO_LOAD,
        IDS_NET_ERROR_HOSTNAME_DOES_NOT_MATCH_CERTIFICATE,
        IDS_NET_ERROR_CONTACT_CONTENT_PROVIDER, IDS_NET_ERROR_EMPTY_STRING,
    },
    {
        net::ERR_CERT_DATE_INVALID,         // -201
        IDS_NET_ERROR_UNABLE_TO_LOAD, IDS_NET_ERROR_CERTIFICATE_HAS_EXPIRED_TV,
        IDS_NET_ERROR_CHECK_TIME_SETTINGS, IDS_NET_ERROR_CURRENT_TIME_SETTINGS,
    },
    {
        net::ERR_CERT_AUTHORITY_INVALID,    // -202
        IDS_NET_ERROR_UNABLE_TO_LOAD,
        IDS_NET_ERROR_CERTIFICATE_CANNOT_BE_TRUSTED,
        IDS_NET_ERROR_CONTACT_CONTENT_PROVIDER, IDS_NET_ERROR_EMPTY_STRING,
    },
    {
        net::ERR_EMPTY_RESPONSE,            // -324
        IDS_NET_ERROR_CHECK_NETWORK_CONNECTION, IDS_NET_ERROR_NETWORK_UNSTABLE,
        IDS_NET_ERROR_CHECK_NETWORK_STATUS_AND_TRY, IDS_NET_ERROR_EMPTY_STRING,
    },
    {
        net::ERR_INSECURE_RESPONSE,         // -501
        IDS_NET_ERROR_CHECK_NETWORK_CONNECTION,
        IDS_NET_ERROR_CERTIFICATE_CANNOT_BE_TRUSTED,
        IDS_NET_ERROR_CONTACT_CONTENT_PROVIDER, IDS_NET_ERROR_EMPTY_STRING,
    },
};

const AppRuntimeLocalizedErrorMap* FindErrorMapInArray(
    const AppRuntimeLocalizedErrorMap* maps, size_t num_maps, int error_code) {
  for (size_t i = 0; i < num_maps; ++i) {
    if (maps[i].error_code == error_code)
      return &maps[i];
  }
  return NULL;
}

const AppRuntimeLocalizedErrorMap* LookupErrorMap(int error_code) {
  return FindErrorMapInArray(net_error_options, std::size(net_error_options),
                             error_code);
}

}  // namespace

void AppRuntimeLocalizedError::GetErrorStrings(int error_code,
    base::Value::Dict& error_strings) {
  // Grab the strings and settings that depend on the error type.  Init
  // options with default values.
  AppRuntimeLocalizedErrorMap options = {
    0,
    IDS_NET_ERROR_CHECK_NETWORK_CONNECTION,
    IDS_NET_ERROR_TEMPORARY_ISSUE,
    IDS_NET_ERROR_CONTACT_CONTENT_PROVIDER,
    IDS_NET_ERROR_TRY_AGAIN_LATER,
  };

  const AppRuntimeLocalizedErrorMap* error_map = LookupErrorMap(error_code);
  if (error_map)
    options = *error_map;

  std::u16string title = l10n_util::GetStringUTF16(options.title_resource_id);
  title += u'(';
  title += base::NumberToString16(error_code);
  title += u')';
  error_strings.Set("error_title", title);
  error_strings.Set("error_details",
                    l10n_util::GetStringUTF16(options.details_resource_id));
  error_strings.Set("error_guide",
                    l10n_util::GetStringUTF16(options.error_guide_resource_id));
  error_strings.Set("error_info",
                    l10n_util::GetStringUTF16(options.error_info_resource_id));
}

}  // namespace neva_app_runtime
