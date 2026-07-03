// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import {CrLitElement} from '//resources/lit/v3_0/lit.rollup.js';

import type {BookmarkNode} from '../bookmarks_api.mojom-webui.js';

import {getCss} from './bookmark_tree_node.css.js';
import {getHtml} from './bookmark_tree_node.html.js';

// TODO(crbug.com/501483829): split this up into two different types.
export class BookmarkTreeNodeElement extends CrLitElement {
  static get is() {
    return 'webui-browser-bookmark-tree-node';
  }

  static override get styles() {
    return getCss();
  }

  override render() {
    return getHtml.bind(this)();
  }

  static override get properties() {
    return {
      node: {type: Object},
    };
  }

  accessor node: BookmarkNode = {
    folder: {
      id: 0n,
      title: '',
      children: [],
    },
  };
}

declare global {
  interface HTMLElementTagNameMap {
    'webui-browser-bookmark-tree-node': BookmarkTreeNodeElement;
  }
}

customElements.define(BookmarkTreeNodeElement.is, BookmarkTreeNodeElement);
