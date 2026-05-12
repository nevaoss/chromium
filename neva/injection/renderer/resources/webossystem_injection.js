// Copyright 2018-2019 LG Electronics, Inc.
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

window.webOSGetResource = function(p1, p2) {
  return webOSSystem.getResource(p1, p2);
};

window.webos = {
  get timezone() {
    return webOSSystem.timeZone;
  }
};

webOSSystem.window.setFocus = function(arg) {
  webOSSystem.window.setProperty(
      'needFocus', arg ? 'true' : 'false');
};

webOSSystem._onCloseWithNotify_ = function(arg) {
  if (typeof(webOSSystem._onCloseCallback_) != 'undefined') {
    if (typeof(webOSSystem._onCloseCallback_) == 'function') {
      webOSSystem._setAppInClosing_();
      webOSSystem._onCloseCallback_(arg);
      if (webOSSystem._didRunOnCloseCallback_() == true)
        webOSSystem.onCloseNotify('didRunOnCloseCallback');
    };
  } else {
    console.log('Callback is undefined');
  };
};

webOSSystem.close = function(p1) {
  if (p1 && p1 == 'EXIT_TO_LASTAPP')
    webOSSystem.window.setProperty(
        '_WEBOS_LAUNCH_PREV_APP_AFTER_CLOSING', 'true');

  if (self !== top)
    top.window.close();
  else
    window.close();
};

Object.defineProperty(webOSSystem, 'onclose', {
  set: function(p1) {
    if (typeof(p1) == 'function') {
      webOSSystem._onCloseCallback_ = p1;
      webOSSystem.onCloseNotify('didSetOnCloseCallback');
    } else if (typeof(p1) === 'undefined' || p1 === undefined) {
      webOSSystem._onCloseCallback_ = p1;
      webOSSystem.onCloseNotify('didClearOnCloseCallback');
    } else {
      console.log('Parameter is not a function');
    };
  },
  get: function() {
    return webOSSystem._onCloseCallback_;
  }
});

window.originalDevicePixelRatio =
    Object.getOwnPropertyDescriptor(window, 'devicePixelRatio');

Object.defineProperty(window, 'devicePixelRatio', {
  get: function() {
    return webOSSystem.devicePixelRatio();
  }
});

// Support PalmSystem for backward compatibility
window.palmGetResource = webOSGetResource;
// Place this code always at the end of injection API
window.PalmSystem = webOSSystem;
