// Copyright 2016-2018 LG Electronics, Inc.
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

#ifndef NEVA_EMULATOR_EMULATOR_EXPORT_H_
#define NEVA_EMULATOR_EMULATOR_EXPORT_H_

#if defined(COMPONENT_BUILD)
#if defined(WIN32)

#if defined(EMULATOR_IMPLEMENTATION)
#define EMULATOR_EXPORT __declspec(dllexport)
#else
#define EMULATOR_EXPORT __declspec(dllimport)
#endif  // defined(EMULATOR_IMPLEMENTATION)

#else  // defined(WIN32)
#if defined(EMULATOR_IMPLEMENTATION)
#define EMULATOR_EXPORT __attribute__((visibility("default")))
#else
#define EMULATOR_EXPORT
#endif  // defined(EMULATOR_IMPLEMENTATION)
#endif

#else  // defined(COMPONENT_BUILD)
#define EMULATOR_EXPORT
#endif

#endif  // NEVA_EMULATOR_EMULATOR_EXPORT_H_
