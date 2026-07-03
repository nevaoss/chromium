// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import {omniboxPopupBrowserProxyFactory, OmniboxPopupPageHandlerRemote, SearchboxBrowserProxy} from 'chrome://omnibox-popup.top-chrome/omnibox_popup.js';
import type {OmniboxPopupPageRemote, OmniboxPopupSearchboxElement} from 'chrome://omnibox-popup.top-chrome/omnibox_popup.js';
import {assertEquals, assertTrue} from 'chrome://webui-test/chai_assert.js';
import {TestMock} from 'chrome://webui-test/test_mock.js';
import {microtasksFinished} from 'chrome://webui-test/test_util.js';

import {TestSearchboxBrowserProxy} from './test_searchbox_browser_proxy.js';

suite('OmniboxPopupSearchboxTest', function() {
  let searchbox: OmniboxPopupSearchboxElement;
  let testProxy: TestSearchboxBrowserProxy;
  let handler: TestMock<OmniboxPopupPageHandlerRemote>&
      OmniboxPopupPageHandlerRemote;
  let callbackRouter: OmniboxPopupPageRemote;

  setup(async () => {
    document.body.innerHTML = window.trustedTypes!.emptyHTML;
    testProxy = new TestSearchboxBrowserProxy();
    SearchboxBrowserProxy.setInstance(testProxy);
    handler = TestMock.fromClass(OmniboxPopupPageHandlerRemote);
    const {instance, remote} =
        omniboxPopupBrowserProxyFactory.createForTest(handler);
    callbackRouter = remote;
    omniboxPopupBrowserProxyFactory.setInstance(instance);
    searchbox = document.createElement('omnibox-popup-searchbox');
    document.body.appendChild(searchbox);
    await microtasksFinished();
  });

  test('HandlesSetInputState', async () => {
    // Set the input state via Mojo.
    const testText = 'test input';
    callbackRouter.setInputState({
      text: testText,
      selection: {start: 0, end: 0},
      sequenceNumber: 1,
    });
    await microtasksFinished();

    // Ensure input element was updated with correct text and selection.
    const lastInput = searchbox.$.input.lastInput();
    assertTrue(!!lastInput);
    assertEquals(testText, lastInput.text);
    const input = searchbox.$.input.inputElement;
    assertEquals(0, input.selectionStart);
    assertEquals(0, input.selectionEnd);
  });

  test('HandlesSelectionChange', async () => {
    // Focus the input so it's the active element.
    const input = searchbox.$.input.inputElement;
    input.focus();
    await microtasksFinished();
    // Set some text in the omnibox popup via Mojo.
    callbackRouter.setInputState({
      text: 'test text',
      selection: {start: 0, end: 0},
      sequenceNumber: 123,
    });
    await microtasksFinished();

    // Set some selection in the HTML.
    input.setSelectionRange(1, 4);
    await microtasksFinished();

    // Ensure handler is notified of the selection change.
    const args = handler.getArgs('onSelectionChanged');
    const state = args[args.length - 1];
    assertEquals('test text', state.text);
    assertEquals(1, state.selection.start);
    assertEquals(4, state.selection.end);
    assertEquals(123, state.sequenceNumber);
  });

  test('IgnoresSelectionChangeWhenNotActive', async () => {
    // Ensure input isn't focused.
    const input = searchbox.$.input.inputElement;
    input.blur();
    await microtasksFinished();

    // Set some text and selection.
    input.value = 'test text';
    input.setSelectionRange(1, 4);
    document.dispatchEvent(new Event('selectionchange'));

    // Ensure handler wasn't notified of the non-active selection change.
    assertEquals(0, handler.getCallCount('onSelectionChanged'));
  });

  test('AppliesSelectionImmediately', async () => {
    // Set some input text and ensure it isn't focused.
    const input = searchbox.$.input.inputElement;
    input.value = 'test text';
    await microtasksFinished();
    input.blur();
    await microtasksFinished();

    // Set the input state via Mojo.
    callbackRouter.setInputState({
      text: 'test text',
      selection: {start: 1, end: 4},
      sequenceNumber: 1,
    });
    await microtasksFinished();

    // Ensure selection was applied immediately regardless of focus.
    assertEquals(1, input.selectionStart);
    assertEquals(4, input.selectionEnd);
  });
});
