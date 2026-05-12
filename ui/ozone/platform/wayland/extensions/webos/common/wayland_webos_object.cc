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

#include "ui/ozone/platform/wayland/extensions/webos/common/wayland_webos_object.h"

#include <wayland-text-client-protocol.h>
#include <wayland-webos-extension-client-protocol.h>
#include <wayland-webos-input-manager-client-protocol.h>
#include <wayland-webos-shell-client-protocol.h>
#include <wayland-webos-surface-group-client-protocol.h>

namespace wl {

const wl_interface* ObjectTraits<text_model>::interface = &text_model_interface;
void (*ObjectTraits<text_model>::deleter)(text_model*) = &text_model_destroy;

const wl_interface* ObjectTraits<text_model_factory>::interface =
    &text_model_factory_interface;
void (*ObjectTraits<text_model_factory>::deleter)(text_model_factory*) =
    &text_model_factory_destroy;

const wl_interface* ObjectTraits<wl_shell>::interface = &wl_shell_interface;
void (*ObjectTraits<wl_shell>::deleter)(wl_shell*) = &wl_shell_destroy;

const wl_interface* ObjectTraits<wl_shell_surface>::interface =
    &wl_shell_surface_interface;
void (*ObjectTraits<wl_shell_surface>::deleter)(wl_shell_surface*) =
    &wl_shell_surface_destroy;

const wl_interface* ObjectTraits<wl_webos_accelerometer>::interface =
    &wl_webos_accelerometer_interface;
void (*ObjectTraits<wl_webos_accelerometer>::deleter)(wl_webos_accelerometer*) =
    &wl_webos_accelerometer_destroy;

const wl_interface* ObjectTraits<wl_webos_gyroscope>::interface =
    &wl_webos_gyroscope_interface;
void (*ObjectTraits<wl_webos_gyroscope>::deleter)(wl_webos_gyroscope*) =
    &wl_webos_gyroscope_destroy;

const wl_interface* ObjectTraits<wl_webos_input_manager>::interface =
    &wl_webos_input_manager_interface;
void (*ObjectTraits<wl_webos_input_manager>::deleter)(wl_webos_input_manager*) =
    &wl_webos_input_manager_destroy;

const wl_interface* ObjectTraits<wl_webos_seat>::interface =
    &wl_webos_seat_interface;
void (*ObjectTraits<wl_webos_seat>::deleter)(wl_webos_seat*) =
    &wl_webos_seat_destroy;

const wl_interface* ObjectTraits<wl_webos_shell>::interface =
    &wl_webos_shell_interface;
void (*ObjectTraits<wl_webos_shell>::deleter)(wl_webos_shell*) =
    &wl_webos_shell_destroy;

const wl_interface* ObjectTraits<wl_webos_shell_surface>::interface =
    &wl_webos_shell_surface_interface;
void (*ObjectTraits<wl_webos_shell_surface>::deleter)(wl_webos_shell_surface*) =
    &wl_webos_shell_surface_destroy;

const wl_interface* ObjectTraits<wl_webos_surface_group>::interface =
    &wl_webos_surface_group_interface;
void (*ObjectTraits<wl_webos_surface_group>::deleter)(wl_webos_surface_group*) =
    &wl_webos_surface_group_destroy;

const wl_interface* ObjectTraits<wl_webos_surface_group_compositor>::interface =
    &wl_webos_surface_group_compositor_interface;
void (*ObjectTraits<wl_webos_surface_group_compositor>::deleter)(
    wl_webos_surface_group_compositor*) =
    &wl_webos_surface_group_compositor_destroy;

const wl_interface* ObjectTraits<wl_webos_surface_group_layer>::interface =
    &wl_webos_surface_group_layer_interface;
void (*ObjectTraits<wl_webos_surface_group_layer>::deleter)(
    wl_webos_surface_group_layer*) = &wl_webos_surface_group_layer_destroy;

const wl_interface* ObjectTraits<wl_webos_xinput>::interface =
    &wl_webos_xinput_interface;
void (*ObjectTraits<wl_webos_xinput>::deleter)(wl_webos_xinput*) =
    &wl_webos_xinput_destroy;

const wl_interface* ObjectTraits<wl_webos_xinput_extension>::interface =
    &wl_webos_xinput_extension_interface;
void (*ObjectTraits<wl_webos_xinput_extension>::deleter)(
    wl_webos_xinput_extension*) = &wl_webos_xinput_extension_destroy;

}  // namespace wl
