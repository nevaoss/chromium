// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import {PageCallbackRouter, PriceTrackingHandlerFactory, PriceTrackingHandlerRemote} from './price_tracking.mojom-webui.js';
import type {PriceTrackingHandlerInterface} from './price_tracking.mojom-webui.js';

let instance: PriceTrackingBrowserProxy|null = null;

export interface PriceTrackingBrowserProxy {
  handler: PriceTrackingHandlerInterface;
  callbackRouter: PageCallbackRouter;
}

export class PriceTrackingBrowserProxyImpl implements
    PriceTrackingBrowserProxy {
  handler: PriceTrackingHandlerRemote;
  callbackRouter: PageCallbackRouter;

  constructor() {
    this.callbackRouter = new PageCallbackRouter();
    this.handler = new PriceTrackingHandlerRemote();

    const factory = PriceTrackingHandlerFactory.getRemote();
    factory.createPriceTrackingHandler(
        this.callbackRouter.$.bindNewPipeAndPassRemote(),
        this.handler.$.bindNewPipeAndPassReceiver());
  }

  static getInstance(): PriceTrackingBrowserProxy {
    return instance || (instance = new PriceTrackingBrowserProxyImpl());
  }

  static setInstance(obj: PriceTrackingBrowserProxy) {
    instance = obj;
  }
}
