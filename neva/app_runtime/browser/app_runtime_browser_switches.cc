// Copyright 2016 LG Electronics, Inc.
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

#include "neva/app_runtime/browser/app_runtime_browser_switches.h"

// Allow file:// access if specified
const char kAllowFileAccess[] = "allow-file-access";

// List of partitions which allow to load extensions
// Must be specifyed with OTR prefix for example "persist:webcontent" or
// "pivate".
// Default partition may be specifyed as empty space and as "persit:default".
const char kAllowLoadExtensionsIn[] = "allow-load-extensions-in";

// Disables modern SameSite cookie behavior. This is the opposite of
// kWebViewEnableModernCookieSameSite flag for android. This disables the
// same-site-by-default-cookies, cookies-without-same-site-must-be-secure,
// and schemeful-same-site features.
const char kDisableModernCookieSameSite[] = "disable-modern-cookie-same-site";

// Forces the maximum disk space to be used by the disk cache, in bytes.
const char kDiskCacheSize[] = "disk-cache-size";

// The GIT revision a target was build from
const char kGitRevision[] = "git-revision";

// Use a specific location for the directory which contains data specific
// to a given user.
const char kUserDataDir[] = "user-data-dir";

// If true devtools experimental settings are enabled
const char kEnableDevToolsExperiments[] = "enable-devtools-experiments";

// Configure the portion of the pool size that can be utilized by a single host
// for temporary storage
const char kPerHostQuotaRatio[] = "per-host-quota-ratio";

// Configure quota pool size ratio for temporary storage such as indexeddb
const char kQuotaPoolSizeRatio[] = "quota-pool-size-ratio";

// Specifies a list of hosts for whom we bypass proxy settings and use direct
// connections. Ignored if --proxy-auto-detect or --no-proxy-server are also
// specified. This is a comma-separated list of bypass rules. See:
// "net/proxy/proxy_bypass_rules.h" for the format of these rules.
const char kProxyBypassList[] = "proxy-bypass-list";

// Disable DropAllPeerConnections call for WebRTC
const char kDisableDropAllPeerConnections[] =
    "disable-drop-all-peer-connections";
