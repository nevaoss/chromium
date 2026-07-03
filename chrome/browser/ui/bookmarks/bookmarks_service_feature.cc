// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/bookmarks/bookmarks_service_feature.h"

#include "components/browser_apis/bookmarks/bookmarks_service_impl.h"

BookmarksServiceFeature::BookmarksServiceFeature(
    bookmarks::BookmarkModel* bookmark_model)
    : bookmarks_service_(std::make_unique<bookmarks_api::BookmarksServiceImpl>(
          bookmark_model)) {}

BookmarksServiceFeature::~BookmarksServiceFeature() = default;

void BookmarksServiceFeature::Accept(
    mojo::PendingReceiver<bookmarks_api::mojom::BookmarksService> receiver) {
  bookmarks_service_->Accept(std::move(receiver));
}
