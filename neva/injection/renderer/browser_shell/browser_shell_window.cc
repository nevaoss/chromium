// Copyright 2021 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include "neva/injection/renderer/browser_shell/browser_shell_window.h"

#include "base/functional/bind.h"
#include "base/notimplemented.h"
#include "gin/dictionary.h"
#include "gin/function_template.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "neva/browser_shell/service/public/mojom/browser_shell_constants.mojom.h"
#include "neva/injection/renderer/browser_shell/browser_shell_page_view.h"
#include "v8/include/cppgc/allocation.h"

namespace injections {

namespace events {
const char kOnBoundsChanged[] = "bounds-changed";
const char kOnDisplaySizeChanged[] = "display-change-size";
const char kVKBChangeState[] = "vkb-change-state";
const char kVKBOverlap[] = "vkb-overlap";
}  // namespace events

char BrowserShellWindow::kPageViewPropertyName[] = "pageView";

char BrowserShellWindow::kAddChildViewMethodName[] = "addChildView";
char BrowserShellWindow::kBringToFrontMethodName[] = "bringToFront";
char BrowserShellWindow::kGetBoundsMethodName[] = "getBounds";
char BrowserShellWindow::kGetChildViewsMethodName[] = "getChildViews";
char BrowserShellWindow::kHasChildViewMethodName[] = "hasChildView";
char BrowserShellWindow::kIsVisibleMethodName[] = "isVisible";
char BrowserShellWindow::kRemoveChildViewMethodName[] = "removeChildView";
char BrowserShellWindow::kSendToBackMethodName[] = "sendToBack";
char BrowserShellWindow::kSetVisibleMethodName[] = "setVisible";

BrowserShellWindow::BrowserShellWindow(
    v8::Isolate* isolate,
    mojo::Remote<browser_shell::mojom::ShellWindow> remote)
    : remote_(std::move(remote)) {
  remote_->RegisterClient(receiver_.BindNewEndpointAndPassRemote());
  mojo::Remote<browser_shell::mojom::PageView> remote_view;
  auto view_pending_receiver = remote_view.BindNewPipeAndPassReceiver();

  BrowserShellPageView::CreateParams page_view_params;
  page_view_params.is_main_view = true;

  auto* shell_page_view =
      cppgc::MakeGarbageCollected<injections::BrowserShellPageView>(
          isolate->GetCppHeap()->GetAllocationHandle(), isolate,
          std::move(remote_view), page_view_params);

  remote_->BindPageView(std::move(view_pending_receiver),
                        base::BindOnce(&injections::BrowserShellPageView::Setup,
                                       shell_page_view->GetAsWeak()));

  page_view_object_.Reset(
      isolate, shell_page_view->GetWrapper(isolate).ToLocalChecked());
}

BrowserShellWindow::~BrowserShellWindow() = default;

void BrowserShellWindow::Setup(const std::string& name,
                               browser_shell::mojom::RectPtr bounds) {
  name_ = name;
  if (!bounds_) {
    bounds_ = std::make_optional<gfx::Rect>(bounds->x, bounds->y, bounds->width,
                                            bounds->height);
  }
}

void BrowserShellWindow::OnBoundsChanged(browser_shell::mojom::RectPtr bounds) {
  bounds_ = std::make_optional<gfx::Rect>(bounds->x, bounds->y, bounds->width,
                                          bounds->height);
  DoEmit(events::kOnBoundsChanged);
}

void BrowserShellWindow::OnDisplaySizeChanged(
    browser_shell::mojom::RectPtr bounds) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Object> wrapper = GetWrapper(isolate).ToLocalChecked();

  v8::Local<v8::Context> context;
  if (!wrapper->GetCreationContext().ToLocal(&context))
    return;

  v8::MicrotasksScope microtasksScope(context,
                                      v8::MicrotasksScope::kRunMicrotasks);
  v8::Context::Scope context_scope(context);
  v8::Local<v8::Object> bounds_object = v8::Object::New(isolate);
  gin::Dictionary bounds_info_dict(isolate, bounds_object);

  bounds_info_dict.Set("x", bounds->x);
  bounds_info_dict.Set("y", bounds->y);
  bounds_info_dict.Set("width", bounds->width);
  bounds_info_dict.Set("height", bounds->height);

  DoEmit(events::kOnDisplaySizeChanged, bounds_object);
}

void BrowserShellWindow::VirtuaKeyboardChangeState(bool visible) {
  DoEmit(events::kVKBChangeState, visible);
}

void BrowserShellWindow::VirtuaKeyboardOverlapTextField(
    browser_shell::mojom::RectPtr bounds) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Object> wrapper = GetWrapper(isolate).ToLocalChecked();

  v8::Local<v8::Context> context;
  if (!wrapper->GetCreationContext().ToLocal(&context))
    return;

  v8::MicrotasksScope microtasksScope(context,
                                      v8::MicrotasksScope::kRunMicrotasks);
  v8::Context::Scope context_scope(context);
  v8::Local<v8::Object> bounds_object = v8::Object::New(isolate);
  gin::Dictionary bounds_info_dict(isolate, bounds_object);

  bounds_info_dict.Set("x", bounds->x);
  bounds_info_dict.Set("y", bounds->y);
  bounds_info_dict.Set("width", bounds->width);
  bounds_info_dict.Set("height", bounds->height);

  DoEmit(events::kVKBOverlap, bounds_object);
}

void BrowserShellWindow::Trace(cppgc::Visitor* visitor) const {
  visitor->Trace(page_view_object_);
  for (const auto& view_object : child_view_objects_) {
    visitor->Trace(view_object.second);
  }

  visitor->Trace(weak_factory_);
  InjectionEventsEmitter<BrowserShellWindow>::Trace(visitor);
  gin::Wrappable<BrowserShellWindow>::Trace(visitor);
}

std::string& BrowserShellWindow::GetName() {
  if (name_.empty() && remote_.is_connected())
    remote_->SyncName(&name_);
  return name_;
}

v8::Local<v8::Object> BrowserShellWindow::GetPageView(v8::Isolate* isolate) {
  return page_view_object_.Get(isolate);
}

void BrowserShellWindow::GetBounds(gin::Arguments* args) {
  if (!bounds_) {
    browser_shell::mojom::RectPtr rect;
    remote_->SyncBounds(&rect);
    bounds_ = std::make_optional<gfx::Rect>(rect->x, rect->y, rect->width,
                                            rect->height);
  }

  v8::Isolate* isolate = args->isolate();
  v8::Local<v8::Object> result = v8::Object::New(isolate);
  gin::Dictionary dict(isolate, result);

  if (dict.Set("x", bounds_->x()) && dict.Set("y", bounds_->y()) &&
      dict.Set("width", bounds_->width()) &&
      dict.Set("height", bounds_->height())) {
    args->Return(result);
  }
}

void BrowserShellWindow::SetVisible(bool visible) {
  NOTIMPLEMENTED();
}

bool BrowserShellWindow::IsVisible() const {
  NOTIMPLEMENTED();
  return true;
}

bool BrowserShellWindow::AddChildView(v8::Isolate* isolate,
                                      v8::Local<v8::Object> object) {
  BrowserShellPageView* page_view = nullptr;
  gin::Converter<BrowserShellPageView*>::FromV8(isolate, object, &page_view);

  if (!page_view || page_view->IsAttached()) {
    return false;
  }

  const uint64_t id = page_view->GetID();
  if (id == 0) {
    return false;
  }

  if (child_view_objects_
          .insert({id, v8::TracedReference<v8::Object>(isolate, object)})
          .second) {
    remote_->AddChildView(id);
    page_view->SetAttached(true);
    return true;
  }
  return false;
}

void BrowserShellWindow::RemoveChildView(v8::Isolate* isolate,
                                         v8::Local<v8::Object> object) {
  BrowserShellPageView* page_view = nullptr;
  gin::Converter<BrowserShellPageView*>::FromV8(isolate, object, &page_view);

  if (page_view) {
    const uint64_t id = page_view->GetID();
    if (id && child_view_objects_.erase(id)) {
      remote_->RemoveChildView(id);
      page_view->SetAttached(false);
    }
  }
}

void BrowserShellWindow::GetChildViews(gin::Arguments* args) {
  v8::Isolate* isolate = args->isolate();
  v8::Local<v8::Set> result = v8::Set::New(isolate);
  for (auto& child_view_item : child_view_objects_) {
    if (!result
             ->Add(isolate->GetCurrentContext(),
                   child_view_item.second.Get(isolate))
             .ToLocal(&result)) {
      return;
    }
  }
  args->Return(result.As<v8::Object>());
}

bool BrowserShellWindow::HasChildView(v8::Isolate* isolate,
                                      v8::Local<v8::Object> object) const {
  BrowserShellPageView* page_view = nullptr;
  gin::Converter<BrowserShellPageView*>::FromV8(isolate, object, &page_view);
  if (page_view) {
    const uint64_t id = page_view->GetID();
    if (id && child_view_objects_.count(id)) {
      return true;
    }
  }
  return false;
}

void BrowserShellWindow::BringToFront(gin::Arguments* args) {
  if (args->Length() == 0) {
    return;
  }

  v8::Local<v8::Object> child;
  if (!args->GetNext(&child)) {
    return;
  }

  BrowserShellPageView* page_view = nullptr;
  gin::Converter<BrowserShellPageView*>::FromV8(args->isolate(), child,
                                                &page_view);

  if (!page_view) {
    return;
  }

  const uint64_t id = page_view->GetID();
  if (id && (child_view_objects_.contains(id) || page_view->IsMainView())) {
    remote_->BringToFrontByID(id);
  }
}

void BrowserShellWindow::SendToBack(gin::Arguments* args) {
  if (args->Length() == 0) {
    return;
  }

  v8::Local<v8::Object> child;
  if (!args->GetNext(&child)) {
    return;
  }

  BrowserShellPageView* page_view = nullptr;
  gin::Converter<BrowserShellPageView*>::FromV8(args->isolate(), child,
                                                &page_view);
  if (!page_view) {
    return;
  }

  const uint64_t id = page_view->GetID();
  if (id && (child_view_objects_.contains(id) || page_view->IsMainView())) {
    remote_->SendToBackByID(id);
  }
}

gin::ObjectTemplateBuilder BrowserShellWindow::GetObjectTemplateBuilder(
    v8::Isolate* isolate) {
  return gin::Wrappable<BrowserShellWindow>::GetObjectTemplateBuilder(isolate)
      .SetProperty(kPageViewPropertyName, &BrowserShellWindow::GetPageView)
      .SetMethod(kEmitMethodName, &BrowserShellWindow::RunEmit)
      .SetMethod(kEventNamesMethodName, &BrowserShellWindow::RunGetEventNames)
      .SetMethod(kListenerCountMethodName,
                 &BrowserShellWindow::RunGetListenerCount)
      .SetMethod(kOnMethodName, &BrowserShellWindow::RunAddEventListener)
      .SetMethod(kOnceMethodName, &BrowserShellWindow::RunAddOnceEventListener)
      .SetMethod(kRemoveEventListenerMethodName,
                 &BrowserShellWindow::RunRemoveEventListener)
      .SetMethod(kRemoveAllEventListenersMethodName,
                 &BrowserShellWindow::RunRemoveAllEventListeners)
      .SetMethod(kGetBoundsMethodName, &BrowserShellWindow::GetBounds)
      .SetMethod(kSetVisibleMethodName, &BrowserShellWindow::SetVisible)
      .SetMethod(kIsVisibleMethodName, &BrowserShellWindow::IsVisible)
      .SetMethod(kAddChildViewMethodName, &BrowserShellWindow::AddChildView)
      .SetMethod(kRemoveChildViewMethodName,
                 &BrowserShellWindow::RemoveChildView)
      .SetMethod(kGetChildViewsMethodName, &BrowserShellWindow::GetChildViews)
      .SetMethod(kHasChildViewMethodName, &BrowserShellWindow::HasChildView)
      .SetMethod(kBringToFrontMethodName, &BrowserShellWindow::BringToFront)
      .SetMethod(kSendToBackMethodName, &BrowserShellWindow::SendToBack);
}

const gin::WrapperInfo* BrowserShellWindow::wrapper_info() const {
  return &kWrapperInfo;
}

}  // namespace injections
