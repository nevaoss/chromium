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

(function () {
  // This is how to reuse the implementation of EventTarget methods from
  // DocumentFragment.
  // To subscribe/unsubscribe to/from a platform event dispatching, native
  // functions implementing the subscription/unsubscription are to be
  // invoked from the EventTarget methods.
  // Wrapper functions mocking some of these EventTarget methods and
  // performing proper native calls are represented below and named as
  // addEventListener and removeEventListener correspondingly.
  var eventTarget = document.createDocumentFragment();
  function createBinding(method) {
      this[method] = eventTarget[method].bind(eventTarget);
  }

  [
    "addEventListener",
    "dispatchEvent",
    "removeEventListener"
  ].forEach(createBinding, navigator.sample);

  // This is how to subscribe to event from native code.
  // For instance:
  //   navigator.sample.addEventListener('samplenotify',eventCallback);
  // where 'eventCallback' is a predefined function.
  var originalAddEventListener = navigator.sample.addEventListener;
  navigator.sample.addEventListener = function(type, listener, useCapture) {
    originalAddEventListener(type, listener, useCapture);
    this._subscribeToEvent();
  };

  // This is how to unsubscribe from event from native code.
  // For instance:
  //   navigator.sample.removeEventListener('samplenotify',eventCallback);
  // where 'eventCallback' is a predefined function.
  var originalRemoveEventListener = navigator.sample.removeEventListener;
  navigator.sample.removeEventListener = function(type, listener, useCapture) {
    originalRemoveEventListener(type, listener, useCapture);
    return this._unsubscribeFromEvent();
  };

  // Here is how to define a property which will return the result of
  // a native function call
  Object.defineProperty(navigator.sample, "value", {
    configurable: false,
    get: function() {
      return this.getValue();
    }
  });

  // This function will also be called when 'onchange' event is fired
  navigator.sample.onchange = function(changeEvent) {};

  // This function will also be called when 'onsamplenotify' event is fired
  navigator.sample.onsamplenotify = function(sampleNotifyEvent) {};

  navigator.sample._receivedSampleUpdate = function(update_value) {
    // Dispatch event
    var sampleNotifyEvent = new CustomEvent(
        'samplenotify', {detail: {value: update_value}});
    this.dispatchEvent(sampleNotifyEvent);

    // Call regular callback if it exists
    if (this.onsamplenotify !== undefined &&
        this.onsamplenotify instanceof Function)
        this.onsamplenotify(sampleNotifyEvent);
  }

  navigator.sample._dispatchValueChanged = function() {
    // Dispatch event
    var changeEvent = new Event("change");
    this.dispatchEvent(changeEvent);
    // Call regular callback if it exists
    if (this.onchange !== undefined &&
        this.onchange instanceof Function)
      this.onchange(changeEvent);
  }
})();
