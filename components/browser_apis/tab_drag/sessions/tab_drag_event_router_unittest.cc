// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/browser_apis/tab_drag/sessions/tab_drag_event_router.h"

#include <memory>
#include <vector>

#include "base/functional/callback_helpers.h"
#include "base/memory/weak_ptr.h"
#include "base/test/run_until.h"
#include "base/test/task_environment.h"
#include "components/browser_apis/tab_drag/adapters/tab_drag_window_adapter.h"
#include "components/browser_apis/tab_drag/sessions/tab_drag_session.h"
#include "components/browser_apis/tab_drag/testing/toy_drop_target.h"
#include "components/browser_apis/tab_drag/testing/toy_tab_drag_session_input_adapter.h"
#include "components/browser_apis/tab_drag/testing/toy_tab_drag_window_adapter.h"
#include "components/browser_apis/tab_strip/types/node_id.h"
#include "mojo/public/cpp/bindings/associated_receiver.h"
#include "mojo/public/cpp/bindings/associated_remote.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/gfx/geometry/point.h"
#include "ui/gfx/geometry/rect.h"

namespace tabs_api {

namespace {

class ToyTabDragSession : public TabDragSession {
 public:
  ToyTabDragSession(const std::vector<tabs_api::NodeId>& dragged_tabs,
                    TabDragWindowAdapter* dragged_window)
      : TabDragSession(dragged_tabs,
                       gfx::Point(),
                       toy_input_adapter_,
                       nullptr,
                       base::OnceClosure()) {
    set_dragged_window(dragged_window);
  }

 private:
  ToyTabDragSessionInputAdapter toy_input_adapter_;
};

}  // namespace

class TabDragEventRouterTest : public ::testing::Test {
 protected:
  TabDragEventRouterTest() = default;
  ~TabDragEventRouterTest() override = default;

  base::test::SingleThreadTaskEnvironment task_environment_;
  TabDragEventRouter router_;
};

TEST_F(TabDragEventRouterTest, RegisterAndUnregister) {
  ToyTabDragWindowAdapter window(gfx::Rect(0, 0, 100, 100));
  ToyDropTarget target;

  mojo::AssociatedRemote<mojom::DropTarget> remote;
  mojo::AssociatedReceiver<mojom::DropTarget> bound_receiver(
      &target, remote.BindNewEndpointAndPassDedicatedReceiver());
  mojo::AssociatedRemote<mojom::DropTargetRegistration> registration;

  router_.RegisterDropTarget(
      &window, remote.Unbind(),
      registration.BindNewEndpointAndPassDedicatedReceiver());

  EXPECT_EQ(1u, router_.drop_targets_count_for_testing());

  // Unregister via registration destruction
  registration.reset();
  ASSERT_TRUE(base::test::RunUntil(
      [&]() { return router_.drop_targets_count_for_testing() == 0u; }));
}

TEST_F(TabDragEventRouterTest, RouteMoveEvents) {
  ToyTabDragWindowAdapter window(gfx::Rect(10, 10, 100, 100));
  ToyDropTarget target;

  mojo::AssociatedRemote<mojom::DropTarget> remote;
  mojo::AssociatedReceiver<mojom::DropTarget> bound_receiver(
      &target, remote.BindNewEndpointAndPassDedicatedReceiver());
  mojo::AssociatedRemote<mojom::DropTargetRegistration> registration;

  router_.RegisterDropTarget(
      &window, remote.Unbind(),
      registration.BindNewEndpointAndPassDedicatedReceiver());

  std::vector<NodeId> tabs = {NodeId(NodeId::Type::kContent, "tab1")};
  ToyTabDragSession session(tabs, nullptr);
  router_.OnSessionStarted(&session);

  // Move inside bounds
  router_.OnDragSessionEvent(
      {TabDragSessionInputEvent::Type::kMoved, gfx::Point(50, 50)});
  ASSERT_TRUE(
      base::test::RunUntil([&]() { return target.events().size() == 1u; }));
  EXPECT_EQ(ToyDropTarget::ReceivedEvent::Type::kEntered,
            target.events()[0].type);
  EXPECT_EQ(gfx::Point(40, 40), target.events()[0].local_point);
  EXPECT_EQ(tabs, target.events()[0].tab_ids);

  // Move again inside bounds
  router_.OnDragSessionEvent(
      {TabDragSessionInputEvent::Type::kMoved, gfx::Point(60, 70)});
  ASSERT_TRUE(
      base::test::RunUntil([&]() { return target.events().size() == 2u; }));
  EXPECT_EQ(ToyDropTarget::ReceivedEvent::Type::kDrag, target.events()[1].type);
  EXPECT_EQ(gfx::Point(50, 60), target.events()[1].local_point);

  // Move outside bounds
  router_.OnDragSessionEvent(
      {TabDragSessionInputEvent::Type::kMoved, gfx::Point(5, 5)});
  ASSERT_TRUE(
      base::test::RunUntil([&]() { return target.events().size() == 3u; }));
  EXPECT_EQ(ToyDropTarget::ReceivedEvent::Type::kLeave,
            target.events()[2].type);

  router_.OnSessionEnded();
}

TEST_F(TabDragEventRouterTest, MultiWindowRouting) {
  ToyTabDragWindowAdapter window_a(gfx::Rect(0, 0, 100, 100));
  ToyDropTarget target_a;
  mojo::AssociatedRemote<mojom::DropTarget> remote_a;
  mojo::AssociatedReceiver<mojom::DropTarget> bound_receiver_a(
      &target_a, remote_a.BindNewEndpointAndPassDedicatedReceiver());
  mojo::AssociatedRemote<mojom::DropTargetRegistration> reg_a;
  router_.RegisterDropTarget(&window_a, remote_a.Unbind(),
                             reg_a.BindNewEndpointAndPassDedicatedReceiver());

  ToyTabDragWindowAdapter window_b(gfx::Rect(200, 0, 100, 100));
  ToyDropTarget target_b;
  mojo::AssociatedRemote<mojom::DropTarget> remote_b;
  mojo::AssociatedReceiver<mojom::DropTarget> bound_receiver_b(
      &target_b, remote_b.BindNewEndpointAndPassDedicatedReceiver());
  mojo::AssociatedRemote<mojom::DropTargetRegistration> reg_b;
  router_.RegisterDropTarget(&window_b, remote_b.Unbind(),
                             reg_b.BindNewEndpointAndPassDedicatedReceiver());

  ToyTabDragSession session({}, nullptr);
  router_.OnSessionStarted(&session);

  // Start in A
  router_.OnDragSessionEvent(
      {TabDragSessionInputEvent::Type::kMoved, gfx::Point(50, 50)});
  ASSERT_TRUE(
      base::test::RunUntil([&]() { return target_a.events().size() == 1u; }));
  EXPECT_EQ(ToyDropTarget::ReceivedEvent::Type::kEntered,
            target_a.events()[0].type);
  EXPECT_EQ(0u, target_b.events().size());

  // Move to B
  router_.OnDragSessionEvent(
      {TabDragSessionInputEvent::Type::kMoved, gfx::Point(250, 50)});
  ASSERT_TRUE(base::test::RunUntil([&]() {
    return target_a.events().size() == 2u && target_b.events().size() == 1u;
  }));

  // A should get Leave
  EXPECT_EQ(ToyDropTarget::ReceivedEvent::Type::kLeave,
            target_a.events()[1].type);

  // B should get Enter
  EXPECT_EQ(ToyDropTarget::ReceivedEvent::Type::kEntered,
            target_b.events()[0].type);
  EXPECT_EQ(gfx::Point(50, 50), target_b.events()[0].local_point);

  router_.OnSessionEnded();
}

TEST_F(TabDragEventRouterTest, IgnoreDraggedWindow) {
  // Window A is the background window
  ToyTabDragWindowAdapter window_a(gfx::Rect(0, 0, 100, 100));
  ToyDropTarget target_a;
  mojo::AssociatedRemote<mojom::DropTarget> remote_a;
  mojo::AssociatedReceiver<mojom::DropTarget> bound_receiver_a(
      &target_a, remote_a.BindNewEndpointAndPassDedicatedReceiver());
  mojo::AssociatedRemote<mojom::DropTargetRegistration> reg_a;
  router_.RegisterDropTarget(&window_a, remote_a.Unbind(),
                             reg_a.BindNewEndpointAndPassDedicatedReceiver());

  // Window B is the window being dragged (overlapping Window A)
  ToyTabDragWindowAdapter window_b(gfx::Rect(0, 0, 100, 100));
  ToyDropTarget target_b;
  mojo::AssociatedRemote<mojom::DropTarget> remote_b;
  mojo::AssociatedReceiver<mojom::DropTarget> bound_receiver_b(
      &target_b, remote_b.BindNewEndpointAndPassDedicatedReceiver());
  mojo::AssociatedRemote<mojom::DropTargetRegistration> reg_b;
  router_.RegisterDropTarget(&window_b, remote_b.Unbind(),
                             reg_b.BindNewEndpointAndPassDedicatedReceiver());

  // Start session dragging Window B
  ToyTabDragSession session({}, &window_b);
  router_.OnSessionStarted(&session);

  // Move over the overlapping area (50, 50)
  router_.OnDragSessionEvent(
      {TabDragSessionInputEvent::Type::kMoved, gfx::Point(50, 50)});
  ASSERT_TRUE(
      base::test::RunUntil([&]() { return target_a.events().size() == 1u; }));
  EXPECT_EQ(ToyDropTarget::ReceivedEvent::Type::kEntered,
            target_a.events()[0].type);

  // Window B should NOT receive the event
  EXPECT_EQ(0u, target_b.events().size());

  router_.OnSessionEnded();
}

TEST_F(TabDragEventRouterTest, DropEvent) {
  ToyTabDragWindowAdapter window(gfx::Rect(0, 0, 100, 100));
  ToyDropTarget target;
  mojo::AssociatedRemote<mojom::DropTarget> remote;
  mojo::AssociatedReceiver<mojom::DropTarget> bound_receiver(
      &target, remote.BindNewEndpointAndPassDedicatedReceiver());
  mojo::AssociatedRemote<mojom::DropTargetRegistration> reg;
  router_.RegisterDropTarget(&window, remote.Unbind(),
                             reg.BindNewEndpointAndPassDedicatedReceiver());

  std::vector<NodeId> tabs = {NodeId(NodeId::Type::kContent, "tab1")};
  ToyTabDragSession session(tabs, nullptr);
  router_.OnSessionStarted(&session);

  // Move in
  router_.OnDragSessionEvent(
      {TabDragSessionInputEvent::Type::kMoved, gfx::Point(50, 50)});
  // Drop
  router_.OnDragSessionEvent(
      {TabDragSessionInputEvent::Type::kDropped, gfx::Point(60, 60)});
  ASSERT_TRUE(
      base::test::RunUntil([&]() { return target.events().size() == 2u; }));
  EXPECT_EQ(ToyDropTarget::ReceivedEvent::Type::kEntered,
            target.events()[0].type);
  EXPECT_EQ(ToyDropTarget::ReceivedEvent::Type::kDrop, target.events()[1].type);
  EXPECT_EQ(gfx::Point(60, 60), target.events()[1].local_point);
  EXPECT_EQ(tabs, target.events()[1].tab_ids);

  router_.OnSessionEnded();
}

TEST_F(TabDragEventRouterTest, CancelEvent) {
  ToyTabDragWindowAdapter window(gfx::Rect(0, 0, 100, 100));
  ToyDropTarget target;
  mojo::AssociatedRemote<mojom::DropTarget> remote;
  mojo::AssociatedReceiver<mojom::DropTarget> bound_receiver(
      &target, remote.BindNewEndpointAndPassDedicatedReceiver());
  mojo::AssociatedRemote<mojom::DropTargetRegistration> reg;
  router_.RegisterDropTarget(&window, remote.Unbind(),
                             reg.BindNewEndpointAndPassDedicatedReceiver());

  ToyTabDragSession session({}, nullptr);
  router_.OnSessionStarted(&session);

  // Move in
  router_.OnDragSessionEvent(
      {TabDragSessionInputEvent::Type::kMoved, gfx::Point(50, 50)});
  // Cancel
  router_.OnDragSessionEvent(
      {TabDragSessionInputEvent::Type::kCancelled, gfx::Point(50, 50)});
  ASSERT_TRUE(
      base::test::RunUntil([&]() { return target.events().size() == 2u; }));
  EXPECT_EQ(ToyDropTarget::ReceivedEvent::Type::kEntered,
            target.events()[0].type);
  EXPECT_EQ(ToyDropTarget::ReceivedEvent::Type::kCancelled,
            target.events()[1].type);

  router_.OnSessionEnded();
}

}  // namespace tabs_api
