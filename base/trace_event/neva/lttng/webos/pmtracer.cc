// Copyright 2022 LG Electronics, Inc.
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

#include "base/trace_event/neva/lttng/webos/pmtracer.h"

#include "base/trace_event/neva/lttng/webos/webossystem_lttng_provider.h"

namespace pmtracer {

void PmTraceTraceMessage(char* label) {
  tracepoint(webossystem_lttng_provider, message, label);
}

void PmTraceTraceItem(char* name, char* value) {
  tracepoint(webossystem_lttng_provider, item, name, value);
}

void PmTraceTraceBefore(char* label) {
  tracepoint(webossystem_lttng_provider, before, label);
}

void PmTraceTraceAfter(char* label) {
  tracepoint(webossystem_lttng_provider, after, label);
}

void PmTraceTraceScopeEntry(char* label) {
  tracepoint(webossystem_lttng_provider, scope_entry, label);
}

void PmTraceTraceScopeExit(char* label) {
  tracepoint(webossystem_lttng_provider, scope_exit, label);
}

void PmTraceTraceFunctionEntry(char* label) {
  tracepoint(webossystem_lttng_provider, function_entry, label);
}

void PmTraceTraceFunctionExit(char* label) {
  tracepoint(webossystem_lttng_provider, function_exit, label);
}

}  // namespace pmtracer
