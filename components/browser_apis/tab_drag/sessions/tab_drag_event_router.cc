// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/browser_apis/tab_drag/sessions/tab_drag_event_router.h"

#include <memory>

#include "base/check.h"
#include "base/notreached.h"
#include "components/browser_apis/tab_drag/adapters/tab_drag_window_adapter.h"
#include "components/browser_apis/tab_drag/sessions/tab_drag_session.h"
#include "mojo/public/cpp/bindings/self_owned_associated_receiver.h"
#include "ui/gfx/geometry/rect.h"

namespace tabs_api {

class DropTargetRegistrationImpl : public mojom::DropTargetRegistration {
 public:
  DropTargetRegistrationImpl(base::WeakPtr<TabDragEventRouter> router,
                             base::WeakPtr<TabDragWindowAdapter> window_adapter)
      : router_(router), window_adapter_(window_adapter) {}

  ~DropTargetRegistrationImpl() override {
    if (router_ && window_adapter_) {
      router_->UnregisterDropTarget(window_adapter_.get());
    }
  }

 private:
  base::WeakPtr<TabDragEventRouter> router_;
  base::WeakPtr<TabDragWindowAdapter> window_adapter_;
};

TabDragEventRouter::TabDragEventRouter() = default;
TabDragEventRouter::~TabDragEventRouter() = default;

void TabDragEventRouter::RegisterDropTarget(
    TabDragWindowAdapter* window_adapter,
    mojo::PendingAssociatedRemote<mojom::DropTarget> target,
    mojo::PendingAssociatedReceiver<mojom::DropTargetRegistration>
        registration) {
  drop_targets_[window_adapter] =
      mojo::AssociatedRemote<mojom::DropTarget>(std::move(target));

  mojo::MakeSelfOwnedAssociatedReceiver(
      std::make_unique<DropTargetRegistrationImpl>(AsWeakPtr(),
                                                   window_adapter->AsWeakPtr()),
      std::move(registration));
}

void TabDragEventRouter::UnregisterDropTarget(
    TabDragWindowAdapter* window_adapter) {
  drop_targets_.erase(window_adapter);
}

void TabDragEventRouter::OnSessionStarted(TabDragSession* session) {
  active_session_ = session;
}

void TabDragEventRouter::OnSessionEnded() {
  active_session_ = nullptr;
  if (current_drop_target_window_) {
    DispatchEvent(current_drop_target_window_, DropTargetEvent::kCancelled);
    current_drop_target_window_ = nullptr;
  }
}

void TabDragEventRouter::OnDragSessionEvent(
    const TabDragSessionInputEvent& event) {
  switch (event.type) {
    case TabDragSessionInputEvent::Type::kMoved:
      HandleDragMoved(event.screen_point);
      break;
    case TabDragSessionInputEvent::Type::kDropped:
      HandleDragDropped(event.screen_point);
      break;
    case TabDragSessionInputEvent::Type::kCancelled:
      HandleDragCancelled();
      break;
  }
}

void TabDragEventRouter::HandleDragMoved(const gfx::Point& screen_point) {
  TabDragWindowAdapter* target_window = FindTargetWindow(screen_point);
  if (target_window != current_drop_target_window_) {
    TransitionToTargetWindow(target_window, screen_point);
  } else if (current_drop_target_window_) {
    DispatchEvent(current_drop_target_window_, DropTargetEvent::kDrag,
                  screen_point);
  }
}

void TabDragEventRouter::HandleDragDropped(const gfx::Point& screen_point) {
  TabDragWindowAdapter* target_window = FindTargetWindow(screen_point);
  if (target_window != current_drop_target_window_) {
    TransitionToTargetWindow(target_window, screen_point);
  }
  if (current_drop_target_window_) {
    DispatchEvent(current_drop_target_window_, DropTargetEvent::kDrop,
                  screen_point);
    current_drop_target_window_ = nullptr;
  }
}

void TabDragEventRouter::HandleDragCancelled() {
  if (current_drop_target_window_) {
    DispatchEvent(current_drop_target_window_, DropTargetEvent::kCancelled);
    current_drop_target_window_ = nullptr;
  }
}

void TabDragEventRouter::TransitionToTargetWindow(
    TabDragWindowAdapter* new_target,
    const gfx::Point& screen_point) {
  if (current_drop_target_window_) {
    DispatchEvent(current_drop_target_window_, DropTargetEvent::kLeave);
  }
  current_drop_target_window_ = new_target;
  if (current_drop_target_window_) {
    DispatchEvent(current_drop_target_window_, DropTargetEvent::kEntered,
                  screen_point);
  }
}

void TabDragEventRouter::DispatchEvent(TabDragWindowAdapter* window,
                                       DropTargetEvent event,
                                       const gfx::Point& screen_point) {
  mojom::DropTarget* target = GetDropTarget(window);
  if (!target) {
    return;
  }

  switch (event) {
    case DropTargetEvent::kEntered:
      target->OnDragEntered(GetDraggedTabs(),
                            window->ConvertScreenPointToLocal(screen_point));
      break;
    case DropTargetEvent::kDrag:
      target->OnDrag(window->ConvertScreenPointToLocal(screen_point));
      break;
    case DropTargetEvent::kLeave:
      target->OnDragLeave();
      break;
    case DropTargetEvent::kDrop:
      target->OnDrop(GetDraggedTabs(),
                     window->ConvertScreenPointToLocal(screen_point));
      break;
    case DropTargetEvent::kCancelled:
      target->OnDragCancelled();
      break;
  }
}

TabDragWindowAdapter* TabDragEventRouter::FindTargetWindow(
    const gfx::Point& screen_point) const {
  CHECK(active_session_);
  TabDragWindowAdapter* dragged_window = active_session_->dragged_window();

  for (const auto& [window_adapter, drop_target] : drop_targets_) {
    if (window_adapter == dragged_window) {
      continue;
    }
    if (window_adapter->GetBoundsInScreen().Contains(screen_point)) {
      return window_adapter;
    }
  }
  return nullptr;
}

mojom::DropTarget* TabDragEventRouter::GetDropTarget(
    TabDragWindowAdapter* window) const {
  auto it = drop_targets_.find(window);
  return it != drop_targets_.end() ? it->second.get() : nullptr;
}

std::vector<tabs_api::NodeId> TabDragEventRouter::GetDraggedTabs() const {
  CHECK(active_session_);
  return active_session_->dragged_tabs();
}

}  // namespace tabs_api
