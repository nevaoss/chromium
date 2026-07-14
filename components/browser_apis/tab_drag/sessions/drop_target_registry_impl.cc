// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/browser_apis/tab_drag/sessions/drop_target_registry_impl.h"

#include <memory>
#include <utility>

#include "components/browser_apis/tab_drag/adapters/tab_drag_window_adapter.h"
#include "components/browser_apis/tab_drag/sessions/drop_target.h"
#include "mojo/public/cpp/bindings/self_owned_associated_receiver.h"

namespace tabs_api {

class DropTargetRegistrationMojoImpl : public mojom::DropTargetRegistration {
 public:
  DropTargetRegistrationMojoImpl(base::WeakPtr<DropTargetRegistryImpl> registry,
                                 DropTargetId target_id)
      : registry_(registry), target_id_(target_id) {}

  ~DropTargetRegistrationMojoImpl() override {
    if (registry_) {
      registry_->UnregisterDropTarget(target_id_);
    }
  }

 private:
  base::WeakPtr<DropTargetRegistryImpl> registry_;
  DropTargetId target_id_;
};

DropTargetRegistryImpl::DropTargetRegistryImpl() = default;
DropTargetRegistryImpl::~DropTargetRegistryImpl() = default;

DropTargetId DropTargetRegistryImpl::RegisterDropTarget(
    TabDragWindowAdapter* window,
    mojo::PendingAssociatedRemote<mojom::DropTarget> target,
    mojo::PendingAssociatedReceiver<mojom::DropTargetRegistration>
        registration) {
  CHECK(window);
  DropTargetId id = id_generator_.GenerateNextId();
  drop_targets_[id] =
      std::make_unique<DropTarget>(id, window, std::move(target));

  mojo::MakeSelfOwnedAssociatedReceiver(
      std::make_unique<DropTargetRegistrationMojoImpl>(AsWeakPtr(), id),
      std::move(registration));
  return id;
}

void DropTargetRegistryImpl::UnregisterDropTarget(DropTargetId target_id) {
  drop_targets_.erase(target_id);
}

DropTargetId DropTargetRegistryImpl::FindTargetAtPoint(
    const gfx::Point& screen_point,
    DropTargetId exclude_target) const {
  for (const auto& [target_id, target] : drop_targets_) {
    if (target_id == exclude_target) {
      continue;
    }
    TabDragWindowAdapter* window_adapter = target->window();
    if (!window_adapter) {
      continue;
    }
    if (window_adapter->GetBoundsInScreen().Contains(screen_point)) {
      return target_id;
    }
  }
  return DropTargetId();
}

DropTargetId DropTargetRegistryImpl::FindTargetForWindow(
    TabDragWindowId window_id) const {
  for (const auto& [target_id, target] : drop_targets_) {
    if (target->window_id() == window_id) {
      return target_id;
    }
  }
  return DropTargetId();
}

DropTarget* DropTargetRegistryImpl::GetDropTarget(
    DropTargetId target_id) const {
  auto it = drop_targets_.find(target_id);
  if (it != drop_targets_.end()) {
    return it->second.get();
  }
  return nullptr;
}

}  // namespace tabs_api
