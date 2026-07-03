// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_BROWSER_APIS_BOOKMARKS_BOOKMARKS_SERVICE_IMPL_H_
#define COMPONENTS_BROWSER_APIS_BOOKMARKS_BOOKMARKS_SERVICE_IMPL_H_

#include "base/memory/raw_ptr.h"
#include "components/browser_apis/bookmarks/bookmarks_service.h"
#include "mojo/public/cpp/bindings/receiver_set.h"

namespace bookmarks {
class BookmarkModel;
class BookmarkNode;
}  // namespace bookmarks

namespace bookmarks_api {

class BookmarksServiceImpl : public BookmarksService {
 public:
  explicit BookmarksServiceImpl(bookmarks::BookmarkModel* bookmark_model);
  BookmarksServiceImpl(const BookmarksServiceImpl&) = delete;
  BookmarksServiceImpl& operator=(const BookmarksServiceImpl&) = delete;
  ~BookmarksServiceImpl() override;

  // BookmarksService:
  void Accept(mojo::PendingReceiver<mojom::BookmarksService> receiver) override;

  // mojom::BookmarksServiceDirectReturnStub:
  mojom::BookmarksService::GetBookmarksResult GetBookmarks() override;
  mojom::BookmarksService::GetBookmarkResult GetBookmark(int64_t id) override;

 private:
  mojom::BookmarkNodePtr ConvertNode(const bookmarks::BookmarkNode* node);

  mojom::BookmarksServiceBridge bridge_{this};

  raw_ptr<bookmarks::BookmarkModel> bookmark_model_;
  mojo::ReceiverSet<mojom::BookmarksService> receivers_;
};

}  // namespace bookmarks_api

#endif  // COMPONENTS_BROWSER_APIS_BOOKMARKS_BOOKMARKS_SERVICE_IMPL_H_
