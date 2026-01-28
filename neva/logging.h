// Copyright 2015-2018 LG Electronics, Inc.
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

#ifndef NEVA_LOGGING_H_
#define NEVA_LOGGING_H_

#include "base/logging.h"

#if defined(DCHECK_ALWAYS_ON)
#define NEVA_DCHECK_ALWAYS_ON 1
#endif

#if defined(NDEBUG) && !defined(NEVA_DCHECK_ALWAYS_ON)
// 'Unforced' release configuration
#define NEVA_DCHECK_IS_ON() 0
#else
// 'Forced' release or debug configuration
#define NEVA_DCHECK_IS_ON() 1
#endif

#define NEVA_DCHECK(condition)                                               \
  LAZY_STREAM(LOG_STREAM(FATAL), NEVA_DCHECK_IS_ON() ? !(condition) : false) \
      << "Check failed: " #condition ". "

// NEVA_LOGs are based on LOG of base/logging.h and it shows additional info on
// the object address of this and the function name of __func__ at the beginning
// of log messages.

// NEVA_LOGTF and NEVA_LOGF uses INFO, WARNING, ERROR, FATAL for log levels.
// NEVA_LOGTF(INFO) << "I'm printed with LOG(INFO)";
// NEVA_LOGF(ERROR) << "I'm printed with LOG(ERROR)";
#define NEVA_LOGTF(x) LOG(x) << "[" << this << "] " << __func__ << ' '

#define NEVA_LOGF(x) LOG(x) << __func__ << ' '

// NEVA_VLOGTF and NEVA_VLOGF use from 0 to 4 for log levels,
// NEVA_VLOGTF(1) << "I'm printed when you run the program with --v=1 or more";
// NEVA_VLOGF(2) << "I'm printed when you run the program with --v=2 or more";
#define NEVA_VLOGTF(x) VLOG(x) << "[" << this << "] " << __func__ << ' '

#define NEVA_VLOGF(x) VLOG(x) << __func__ << ' '

// NEVA_DLOGTF, NEVA_DLOGF, NEVA_DVLOGTF, and NEVA_DVLOGF are compiled away to
// nothing for not-debug mode
#define NEVA_DLOGTF(x) DLOG(x) << "[" << this << "] " << __func__ << ' '

#define NEVA_DLOGF(x) DLOG(x) << __func__ << ' '

#define NEVA_DVLOGTF(x) DVLOG(x) << "[" << this << "] " << __func__ << ' '

#define NEVA_DVLOGF(x) DVLOG(x) << __func__ << ' '

#endif  // NEVA_LOGGING_H_
