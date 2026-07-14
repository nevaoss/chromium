// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_BROWSER_APIS_TAB_DRAG_SESSIONS_DROP_TARGET_H_
#define COMPONENTS_BROWSER_APIS_TAB_DRAG_SESSIONS_DROP_TARGET_H_

#include <vector>

#include "base/memory/raw_ptr.h"
#include "components/browser_apis/tab_drag/adapters/tab_drag_window_adapter.h"
#include "components/browser_apis/tab_drag/sessions/drop_target_id.h"
#include "components/browser_apis/tab_drag/tab_drag_api.mojom.h"
#include "mojo/public/cpp/bindings/associated_remote.h"
#include "mojo/public/cpp/bindings/pending_associated_remote.h"

namespace gfx {
class Point;
}

namespace tabs_api {

class DropTarget {
 public:
  DropTarget(DropTargetId id,
             TabDragWindowAdapter* window,
             mojo::PendingAssociatedRemote<mojom::DropTarget> remote);
  DropTarget(const DropTarget&) = delete;
  DropTarget& operator=(const DropTarget&) = delete;
  ~DropTarget();

  DropTargetId id() const { return id_; }
  TabDragWindowId window_id() const { return window_->GetWindowId(); }
  TabDragWindowAdapter* window() const { return window_.get(); }

  // Forward drag events to the underlying Mojo remote, performing coordinate
  // conversion internally using the window handle.
  void DragEnter(const std::vector<tabs_api::NodeId>& dragged_tabs,
                 const gfx::Point& screen_point);
  void DragOver(const gfx::Point& screen_point);
  void DragLeave();
  void DragCancel();
  void Drop(const std::vector<tabs_api::NodeId>& dragged_tabs,
            const gfx::Point& screen_point);

 private:
  const DropTargetId id_;
  const raw_ptr<TabDragWindowAdapter> window_;
  mojo::AssociatedRemote<mojom::DropTarget> remote_;
};

}  // namespace tabs_api

#endif  // COMPONENTS_BROWSER_APIS_TAB_DRAG_SESSIONS_DROP_TARGET_H_
