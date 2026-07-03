// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_BOOKMARKS_BOOKMARKS_SERVICE_FEATURE_H_
#define CHROME_BROWSER_UI_BOOKMARKS_BOOKMARKS_SERVICE_FEATURE_H_

#include <memory>

#include "components/browser_apis/bookmarks/bookmarks_api.mojom.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"

namespace bookmarks {
class BookmarkModel;
}

namespace bookmarks_api {
class BookmarksService;
}

class BookmarksServiceFeature {
 public:
  explicit BookmarksServiceFeature(bookmarks::BookmarkModel* bookmark_model);
  ~BookmarksServiceFeature();

  void Accept(
      mojo::PendingReceiver<bookmarks_api::mojom::BookmarksService> receiver);

 private:
  std::unique_ptr<bookmarks_api::BookmarksService> bookmarks_service_;
};

#endif  // CHROME_BROWSER_UI_BOOKMARKS_BOOKMARKS_SERVICE_FEATURE_H_
