// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/browser_apis/tab_drag/testing/toy_drop_target_registry.h"

#include <utility>

#include "components/browser_apis/tab_drag/adapters/tab_drag_window_adapter.h"
#include "components/browser_apis/tab_drag/sessions/drop_target.h"
#include "components/browser_apis/tab_drag/testing/toy_tab_drag_window_adapter.h"
#include "ui/gfx/geometry/point.h"

namespace tabs_api {

ToyDropTargetRegistry::ToyDropTargetRegistry() : target_id_(1) {}
ToyDropTargetRegistry::~ToyDropTargetRegistry() = default;

DropTargetId ToyDropTargetRegistry::RegisterDropTarget(
    TabDragWindowAdapter* window,
    mojo::PendingAssociatedRemote<mojom::DropTarget> target,
    mojo::PendingAssociatedReceiver<mojom::DropTargetRegistration>
        registration) {
  return DropTargetId();
}

void ToyDropTargetRegistry::UnregisterDropTarget(DropTargetId target_id) {}

DropTargetId ToyDropTargetRegistry::FindTargetAtPoint(
    const gfx::Point& screen_point,
    DropTargetId exclude_target) const {
  if (target_window_ && exclude_target != target_id_) {
    return target_id_;
  }
  return DropTargetId();
}

DropTargetId ToyDropTargetRegistry::FindTargetForWindow(
    TabDragWindowId window_id) const {
  if (target_window_ && target_window_->GetWindowId() == window_id) {
    return target_id_;
  }
  if (source_window_ && source_window_->GetWindowId() == window_id) {
    return source_id_;
  }
  return DropTargetId();
}

DropTarget* ToyDropTargetRegistry::GetDropTarget(DropTargetId target_id) const {
  if (target_id == target_id_ && target_window_) {
    if (!target_object_ ||
        target_object_->window_id() != target_window_->GetWindowId()) {
      mojo::PendingAssociatedRemote<mojom::DropTarget> null_remote;
      target_object_ = std::make_unique<DropTarget>(target_id_, target_window_,
                                                    std::move(null_remote));
    }
    return target_object_.get();
  }
  return nullptr;
}

void ToyDropTargetRegistry::set_target_window(ToyTabDragWindowAdapter* window) {
  target_window_ = window;
}

void ToyDropTargetRegistry::set_source_window(ToyTabDragWindowAdapter* window) {
  source_window_ = window;
}

}  // namespace tabs_api
