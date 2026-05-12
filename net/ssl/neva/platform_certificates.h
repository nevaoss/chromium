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

#ifndef NET_SSL_NEVA_PLATFORM_CERTIFICATES_H_
#define NET_SSL_NEVA_PLATFORM_CERTIFICATES_H_

namespace net {

namespace neva {

int MultipleReadClientKey(char*** pPub,
                          char*** pPrv,
                          char*** pCA,
                          int* pNum);

}  // namespace neva

void EnsurePlatformCerts();

}  // namespace net

#endif  // NET_SSL_NEVA_PLATFORM_CERTIFICATES_H_
