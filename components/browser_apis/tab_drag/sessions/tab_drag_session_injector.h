// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_BROWSER_APIS_TAB_DRAG_SESSIONS_TAB_DRAG_SESSION_INJECTOR_H_
#define COMPONENTS_BROWSER_APIS_TAB_DRAG_SESSIONS_TAB_DRAG_SESSION_INJECTOR_H_

#include <functional>
#include <optional>

#include "components/browser_apis/tab_drag/adapters/tab_drag_window_adapter.h"
#include "components/browser_apis/tab_drag/sessions/drop_target_id.h"
#include "components/browser_apis/tab_drag/tab_drag_api.mojom-forward.h"
#include "mojo/public/cpp/bindings/pending_associated_receiver.h"
#include "mojo/public/cpp/bindings/pending_associated_remote.h"

namespace tabs_api {

class TabDragSessionInputAdapter;
class TabDragSessionListener;
class DropTarget;
class TabDragWindowRegistry;

class DropTargetRegistry {
 public:
  virtual ~DropTargetRegistry() = default;
  // Registers a `target` (DropTarget) associated with the given
  // `window`. The `registration` receiver is used to manage the lifetime
  // of the registration. Returns a unique DropTargetId for this registration.
  virtual DropTargetId RegisterDropTarget(
      TabDragWindowAdapter* window,
      mojo::PendingAssociatedRemote<mojom::DropTarget> target,
      mojo::PendingAssociatedReceiver<mojom::DropTargetRegistration>
          registration) = 0;

  // Unregisters the drop target associated with the given `target_id`.
  virtual void UnregisterDropTarget(DropTargetId target_id) = 0;

  // Returns the ID of the drop target under `screen_point`, excluding
  // `exclude_target`.
  virtual DropTargetId FindTargetAtPoint(const gfx::Point& screen_point,
                                         DropTargetId exclude_target) const = 0;

  // Returns the ID of the drop target associated with `window_id`, or Null
  // if not found.
  virtual DropTargetId FindTargetForWindow(TabDragWindowId window_id) const = 0;

  // Returns the C++ DropTarget object for the given ID, or nullptr if not
  // found.
  virtual DropTarget* GetDropTarget(DropTargetId target_id) const = 0;
};

class TabDragSessionInjector {
 public:
  virtual ~TabDragSessionInjector() = default;

  virtual TabDragWindowRegistry* GetWindowRegistry() = 0;
  virtual TabDragSessionInputAdapter& GetInputAdapter() = 0;
  virtual TabDragSessionListener& GetSessionListener() = 0;
  virtual DropTargetRegistry& GetDropTargetRegistry() = 0;
};

}  // namespace tabs_api

#endif  // COMPONENTS_BROWSER_APIS_TAB_DRAG_SESSIONS_TAB_DRAG_SESSION_INJECTOR_H_
