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

#ifndef UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_COMMON_WAYLAND_WEBOS_OBJECT_H_
#define UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_COMMON_WAYLAND_WEBOS_OBJECT_H_

#include "ui/ozone/platform/wayland/common/wayland_object.h"

struct text_model;
struct text_model_factory;
struct wl_shell;
struct wl_shell_surface;
struct wl_webos_accelerometer;
struct wl_webos_gyroscope;
struct wl_webos_input_manager;
struct wl_webos_seat;
struct wl_webos_shell;
struct wl_webos_shell_surface;
struct wl_webos_surface_group;
struct wl_webos_surface_group_compositor;
struct wl_webos_surface_group_layer;
struct wl_webos_xinput;
struct wl_webos_xinput_extension;

namespace wl {

template <>
struct ObjectTraits<text_model> {
  static const wl_interface* interface;
  static void (*deleter)(text_model*);
};

template <>
struct ObjectTraits<text_model_factory> {
  static const wl_interface* interface;
  static void (*deleter)(text_model_factory*);
};

template <>
struct ObjectTraits<wl_shell> {
  static const wl_interface* interface;
  static void (*deleter)(wl_shell*);
};

template <>
struct ObjectTraits<wl_shell_surface> {
  static const wl_interface* interface;
  static void (*deleter)(wl_shell_surface*);
};

template <>
struct ObjectTraits<wl_webos_accelerometer> {
  static const wl_interface* interface;
  static void (*deleter)(wl_webos_accelerometer*);
};

template <>
struct ObjectTraits<wl_webos_gyroscope> {
  static const wl_interface* interface;
  static void (*deleter)(wl_webos_gyroscope*);
};

template <>
struct ObjectTraits<wl_webos_input_manager> {
  static const wl_interface* interface;
  static void (*deleter)(wl_webos_input_manager*);
};

template <>
struct ObjectTraits<wl_webos_seat> {
  static const wl_interface* interface;
  static void (*deleter)(wl_webos_seat*);
};

template <>
struct ObjectTraits<wl_webos_shell> {
  static const wl_interface* interface;
  static void (*deleter)(wl_webos_shell*);
};

template <>
struct ObjectTraits<wl_webos_shell_surface> {
  static const wl_interface* interface;
  static void (*deleter)(wl_webos_shell_surface*);
};

template <>
struct ObjectTraits<wl_webos_surface_group> {
  static const wl_interface* interface;
  static void (*deleter)(wl_webos_surface_group*);
};

template <>
struct ObjectTraits<wl_webos_surface_group_compositor> {
  static const wl_interface* interface;
  static void (*deleter)(wl_webos_surface_group_compositor*);
};

template <>
struct ObjectTraits<wl_webos_surface_group_layer> {
  static const wl_interface* interface;
  static void (*deleter)(wl_webos_surface_group_layer*);
};

template <>
struct ObjectTraits<wl_webos_xinput> {
  static const wl_interface* interface;
  static void (*deleter)(wl_webos_xinput*);
};

template <>
struct ObjectTraits<wl_webos_xinput_extension> {
  static const wl_interface* interface;
  static void (*deleter)(wl_webos_xinput_extension*);
};

}  // namespace wl

#endif  // UI_OZONE_PLATFORM_WAYLAND_EXTENSIONS_WEBOS_COMMON_WAYLAND_WEBOS_OBJECT_H_
