// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/browser_apis/bookmarks/bookmarks_service_impl.h"

#include "base/strings/utf_string_conversions.h"
#include "components/bookmarks/browser/bookmark_model.h"
#include "components/bookmarks/browser/bookmark_node.h"
#include "components/bookmarks/browser/bookmark_utils.h"

namespace bookmarks_api {

BookmarksServiceImpl::BookmarksServiceImpl(
    bookmarks::BookmarkModel* bookmark_model)
    : bookmark_model_(bookmark_model) {
  CHECK(bookmark_model_);
}

BookmarksServiceImpl::~BookmarksServiceImpl() = default;

void BookmarksServiceImpl::Accept(
    mojo::PendingReceiver<mojom::BookmarksService> receiver) {
  receivers_.Add(&bridge_, std::move(receiver));
}

mojom::BookmarksService::GetBookmarksResult
BookmarksServiceImpl::GetBookmarks() {
  if (!bookmark_model_ || !bookmark_model_->loaded()) {
    return base::unexpected(mojo_base::mojom::Error::New(
        mojo_base::mojom::Code::kFailedPrecondition,
        "Bookmark model not loaded"));
  }
  return ConvertNode(bookmark_model_->root_node());
}

mojom::BookmarksService::GetBookmarkResult BookmarksServiceImpl::GetBookmark(
    int64_t id) {
  if (!bookmark_model_ || !bookmark_model_->loaded()) {
    return base::unexpected(mojo_base::mojom::Error::New(
        mojo_base::mojom::Code::kFailedPrecondition,
        "Bookmark model not loaded"));
  }
  const bookmarks::BookmarkNode* node =
      bookmarks::GetBookmarkNodeByID(bookmark_model_, id);
  if (!node) {
    return base::unexpected(mojo_base::mojom::Error::New(
        mojo_base::mojom::Code::kNotFound, "Bookmark not found"));
  }
  return ConvertNode(node);
}

mojom::BookmarkNodePtr BookmarksServiceImpl::ConvertNode(
    const bookmarks::BookmarkNode* node) {
  if (node->is_url()) {
    auto url_node = mojom::Url::New();
    url_node->id = node->id();
    url_node->title = base::UTF16ToUTF8(node->GetTitle());
    url_node->url = node->url();
    return mojom::BookmarkNode::NewUrl(std::move(url_node));
  } else {
    auto folder_node = mojom::Folder::New();
    folder_node->id = node->id();
    folder_node->title = base::UTF16ToUTF8(node->GetTitle());
    for (const auto& child : node->children()) {
      folder_node->children.push_back(ConvertNode(child.get()));
    }
    return mojom::BookmarkNode::NewFolder(std::move(folder_node));
  }
}

}  // namespace bookmarks_api
