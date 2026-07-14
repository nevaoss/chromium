// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/browser_apis/tab_drag/sessions/tab_drag_session.h"

#include <functional>
#include <memory>
#include <optional>
#include <vector>

#include "base/functional/callback.h"
#include "base/memory/raw_ptr.h"
#include "base/test/mock_callback.h"
#include "components/browser_apis/tab_drag/adapters/tab_drag_session_input_adapter.h"
#include "components/browser_apis/tab_drag/sessions/drop_target.h"
#include "components/browser_apis/tab_drag/sessions/tab_drag_session_injector.h"
#include "components/browser_apis/tab_drag/sessions/tab_drag_session_listener.h"
#include "components/browser_apis/tab_drag/sessions/tab_drag_window_registry.h"
#include "components/browser_apis/tab_drag/testing/toy_drop_target_registry.h"
#include "components/browser_apis/tab_drag/testing/toy_tab_drag_session_input_adapter.h"
#include "components/browser_apis/tab_drag/testing/toy_tab_drag_session_listener.h"
#include "components/browser_apis/tab_drag/testing/toy_tab_drag_window_adapter.h"
#include "components/browser_apis/tab_strip/types/node_id.h"
#include "mojo/public/mojom/base/error.mojom.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/gfx/geometry/point.h"
#include "ui/gfx/geometry/vector2d.h"

namespace tabs_api {

class TabDragSessionTest : public ::testing::Test {
 protected:
  TabDragSessionTest() : dummy_window_(gfx::Rect(0, 0, 100, 100), &registry_) {}
  ~TabDragSessionTest() override = default;

  TabDragWindowRegistry registry_;
  ToyTabDragWindowAdapter dummy_window_;
};


TEST_F(TabDragSessionTest, StartAndReleaseCapture) {
  ToyTabDragSessionInputAdapter toy_adapter;
  ToyTabDragSessionListener listener;
  ToyDropTargetRegistry dummy_registry;
  TabDragWindowRegistry registry;
  ToyTabDragSessionInjector injector(toy_adapter, listener, dummy_registry,
                                     &registry);
  base::MockOnceClosure end_callback;
  ToyTabDragWindowAdapter toy_window(gfx::Rect(0, 0, 100, 100), &registry);

  EXPECT_FALSE(toy_adapter.capture_started());
  EXPECT_FALSE(toy_adapter.capture_released());
  EXPECT_FALSE(toy_window.HasCapture());

  {
    TabDragSessionParams params{
        .source_window_id = toy_window.GetWindowId(),
        .source_tab_ids = {NodeId(NodeId::Type::kContent, "tab1")},
        .start_point = gfx::Point(),
        .end_callback = end_callback.Get()};
    TabDragSession session(std::move(params), &injector);
    EXPECT_FALSE(toy_adapter.capture_started());
    EXPECT_TRUE(session.Start().has_value());
    EXPECT_TRUE(toy_adapter.capture_started());
    EXPECT_FALSE(toy_adapter.capture_released());
    EXPECT_TRUE(toy_window.HasCapture());
  }

  EXPECT_TRUE(toy_adapter.capture_released());
  EXPECT_FALSE(toy_window.HasCapture());
}

TEST_F(TabDragSessionTest, InputEventCancelled) {
  ToyTabDragSessionInputAdapter toy_adapter;
  ToyTabDragSessionListener listener;
  ToyDropTargetRegistry dummy_registry;
  ToyTabDragSessionInjector injector(toy_adapter, listener, dummy_registry,
                                     &registry_);
  base::MockOnceClosure end_callback;

  TabDragSessionParams params{
      .source_window_id = dummy_window_.GetWindowId(),
      .source_tab_ids = {NodeId(NodeId::Type::kContent, "tab1")},
      .start_point = gfx::Point(),
      .end_callback = end_callback.Get()};
  TabDragSession session(std::move(params), &injector);
  EXPECT_TRUE(session.Start().has_value());

  EXPECT_CALL(end_callback, Run()).Times(1);
  toy_adapter.SendToyEvent(TabDragInputEvent::Type::kCancelled);
}

TEST_F(TabDragSessionTest, InputEventDropped) {
  ToyTabDragSessionInputAdapter toy_adapter;
  ToyTabDragSessionListener listener;
  ToyDropTargetRegistry dummy_registry;
  ToyTabDragSessionInjector injector(toy_adapter, listener, dummy_registry,
                                     &registry_);
  base::MockOnceClosure end_callback;

  TabDragSessionParams params{
      .source_window_id = dummy_window_.GetWindowId(),
      .source_tab_ids = {NodeId(NodeId::Type::kContent, "tab1")},
      .start_point = gfx::Point(),
      .end_callback = end_callback.Get()};
  TabDragSession session(std::move(params), &injector);
  EXPECT_TRUE(session.Start().has_value());

  EXPECT_CALL(end_callback, Run()).Times(1);
  toy_adapter.SendToyEvent(TabDragInputEvent::Type::kDropped);
}

TEST_F(TabDragSessionTest, CoordinateTracking) {
  ToyTabDragSessionInputAdapter toy_adapter;
  ToyTabDragSessionListener listener;
  ToyDropTargetRegistry dummy_registry;
  ToyTabDragSessionInjector injector(toy_adapter, listener, dummy_registry,
                                     &registry_);
  base::MockOnceClosure end_callback;

  gfx::Point start_point(10, 10);
  TabDragSessionParams params{
      .source_window_id = dummy_window_.GetWindowId(),
      .source_tab_ids = {NodeId(NodeId::Type::kContent, "tab1")},
      .start_point = start_point,
      .end_callback = end_callback.Get()};
  TabDragSession session(std::move(params), &injector);
  EXPECT_TRUE(session.Start().has_value());

  EXPECT_EQ(session.start_point_in_screen(), start_point);
  EXPECT_EQ(session.last_mouse_screen_point(), start_point);
  EXPECT_EQ(session.delta(), gfx::Vector2d(0, 0));

  // Move mouse
  gfx::Point move_point(15, 20);
  toy_adapter.SendToyEvent(TabDragInputEvent::Type::kMoved, move_point);

  EXPECT_EQ(session.last_mouse_screen_point(), move_point);
  EXPECT_EQ(session.delta(), gfx::Vector2d(5, 10));

  // Drop mouse
  gfx::Point drop_point(25, 30);
  EXPECT_CALL(end_callback, Run()).Times(1);
  toy_adapter.SendToyEvent(TabDragInputEvent::Type::kDropped, drop_point);

  EXPECT_EQ(session.last_mouse_screen_point(), drop_point);
  EXPECT_EQ(session.delta(), gfx::Vector2d(15, 20));
}

TEST_F(TabDragSessionTest, ListenerNotification) {
  ToyTabDragSessionInputAdapter toy_adapter;
  base::MockOnceClosure end_callback;
  ToyTabDragSessionListener listener;
  ToyDropTargetRegistry registry;
  registry.set_source_window(&dummy_window_);
  ToyTabDragSessionInjector injector(toy_adapter, listener, registry,
                                     &registry_);
  ToyTabDragWindowAdapter target_window(gfx::Rect(0, 0, 100, 100), &registry_);

  std::vector<tabs_api::NodeId> tab_ids = {
      NodeId(NodeId::Type::kContent, "tab1")};
  TabDragSessionParams params{.source_window_id = dummy_window_.GetWindowId(),
                              .source_tab_ids = tab_ids,
                              .start_point = gfx::Point(),
                              .end_callback = end_callback.Get()};
  TabDragSession session(std::move(params), &injector);

  ASSERT_EQ(listener.events().size(), 0u);
  EXPECT_TRUE(session.Start().has_value());
  ASSERT_EQ(listener.events().size(), 1u);
  EXPECT_EQ(listener.events()[0].type,
            ToyTabDragSessionListener::Event::Type::kStarted);
  EXPECT_EQ(listener.events()[0].dragged_tabs, tab_ids);
  EXPECT_EQ(listener.events()[0].window_id, dummy_window_.GetWindowId());
  EXPECT_EQ(listener.events()[0].point, gfx::Point());

  // Move outside source window to trigger tear-off.
  // dummy_window_ is (0,0, 100,100). Threshold is 15.
  gfx::Point tear_point(120, 120);
  toy_adapter.SendToyEvent(TabDragInputEvent::Type::kMoved, tear_point);

  // No event is fired during tear-off in the simplified design.
  ASSERT_EQ(listener.events().size(), 1u);

  // Move to a target window (simulate merge)
  registry.set_target_window(&target_window);
  gfx::Point move_point1(10, 20);
  toy_adapter.SendToyEvent(TabDragInputEvent::Type::kMoved, move_point1);

  ASSERT_EQ(listener.events().size(), 2u);
  EXPECT_EQ(listener.events()[1].type,
            ToyTabDragSessionListener::Event::Type::kTargetChanged);
  EXPECT_EQ(listener.events()[1].target, registry.target_id());
  EXPECT_EQ(listener.events()[1].point, move_point1);

  // Move within the target window
  gfx::Point move_point2(15, 25);
  toy_adapter.SendToyEvent(TabDragInputEvent::Type::kMoved, move_point2);

  ASSERT_EQ(listener.events().size(), 3u);
  EXPECT_EQ(listener.events()[2].type,
            ToyTabDragSessionListener::Event::Type::kMoved);
  EXPECT_EQ(listener.events()[2].point, move_point2);

  // Drop
  gfx::Point drop_point(30, 40);
  EXPECT_CALL(end_callback, Run()).Times(1);
  toy_adapter.SendToyEvent(TabDragInputEvent::Type::kDropped, drop_point);

  // Since target didn't change at drop, we just get OnSessionDropped
  ASSERT_EQ(listener.events().size(), 4u);
  EXPECT_EQ(listener.events()[3].type,
            ToyTabDragSessionListener::Event::Type::kDropped);
  EXPECT_EQ(listener.events()[3].point, drop_point);
}

TEST_F(TabDragSessionTest, CaptureLostExternally) {
  ToyTabDragSessionInputAdapter toy_adapter;
  ToyTabDragSessionListener listener;
  ToyDropTargetRegistry registry;
  TabDragWindowRegistry window_registry;
  ToyTabDragSessionInjector injector(toy_adapter, listener, registry,
                                     &window_registry);
  base::MockOnceClosure end_callback;
  ToyTabDragWindowAdapter toy_window(gfx::Rect(0, 0, 100, 100),
                                     &window_registry);

  TabDragSessionParams params{
      .source_window_id = toy_window.GetWindowId(),
      .source_tab_ids = {NodeId(NodeId::Type::kContent, "tab1")},
      .start_point = gfx::Point(),
      .end_callback = end_callback.Get()};
  TabDragSession session(std::move(params), &injector);
  EXPECT_TRUE(session.Start().has_value());
  EXPECT_TRUE(toy_window.HasCapture());

  // Simulate external capture loss.
  toy_window.ReleaseCapture();
  EXPECT_CALL(end_callback, Run()).Times(1);
  toy_adapter.SendToyEvent(TabDragInputEvent::Type::kCaptureChanged);
}

}  // namespace tabs_api
