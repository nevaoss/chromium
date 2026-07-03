// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/browser_apis/bookmarks/bookmarks_service_impl.h"

#include "base/test/task_environment.h"
#include "components/bookmarks/browser/bookmark_model.h"
#include "components/bookmarks/test/bookmark_test_helpers.h"
#include "components/bookmarks/test/test_bookmark_client.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace bookmarks_api {

class BookmarksServiceImplTest : public testing::Test {
 protected:
  BookmarksServiceImplTest() {
    model_ = std::make_unique<bookmarks::BookmarkModel>(
        std::make_unique<bookmarks::TestBookmarkClient>());
    model_->LoadEmptyForTest();
    service_ = std::make_unique<BookmarksServiceImpl>(model_.get());
  }

  base::test::TaskEnvironment task_environment_;
  std::unique_ptr<bookmarks::BookmarkModel> model_;
  std::unique_ptr<BookmarksServiceImpl> service_;
};

TEST_F(BookmarksServiceImplTest, GetBookmarks_NotLoaded) {
  auto model = std::make_unique<bookmarks::BookmarkModel>(
      std::make_unique<bookmarks::TestBookmarkClient>());
  BookmarksServiceImpl service(model.get());
  auto result = service.GetBookmarks();
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error()->code, mojo_base::mojom::Code::kFailedPrecondition);
}

TEST_F(BookmarksServiceImplTest, GetBookmarks_Empty) {
  auto result = service_->GetBookmarks();
  ASSERT_TRUE(result.has_value());
  ASSERT_TRUE(result.value()->is_folder());
  const auto& root_folder = result.value()->get_folder();
  EXPECT_EQ(root_folder->id, model_->root_node()->id());

  ASSERT_EQ(root_folder->children.size(), 3u);
  ASSERT_TRUE(root_folder->children[0]->is_folder());
  EXPECT_TRUE(root_folder->children[0]->get_folder()->children.empty());
  ASSERT_TRUE(root_folder->children[1]->is_folder());
  EXPECT_TRUE(root_folder->children[1]->get_folder()->children.empty());
  ASSERT_TRUE(root_folder->children[2]->is_folder());
  EXPECT_TRUE(root_folder->children[2]->get_folder()->children.empty());
}

TEST_F(BookmarksServiceImplTest, GetBookmark_NotFound) {
  auto result = service_->GetBookmark(999);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error()->code, mojo_base::mojom::Code::kNotFound);
}

TEST_F(BookmarksServiceImplTest, GetBookmark_Success) {
  const bookmarks::BookmarkNode* parent = model_->bookmark_bar_node();
  ASSERT_TRUE(parent);
  const bookmarks::BookmarkNode* node =
      model_->AddURL(parent, 0, u"Title", GURL("http://example.com"));

  auto result = service_->GetBookmark(node->id());
  ASSERT_TRUE(result.has_value());
  ASSERT_TRUE(result.value()->is_url());
  const auto& url_node = result.value()->get_url();
  EXPECT_EQ(url_node->id, node->id());
  EXPECT_EQ(url_node->title, "Title");
  EXPECT_EQ(url_node->url, GURL("http://example.com"));
}

}  // namespace bookmarks_api
