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
#include "gin/dictionary.h"
#include "gin/function_template.h"
#include "gin/handle.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "neva/browser_shell/service/public/mojom/browser_shell_constants.mojom.h"
#include "neva/injection/renderer/browser_shell/browser_shell_page_view.h"

namespace injections {

namespace events {
const char kOnDisplaySizeChanged[] = "display-change-size";
const char kVKBChangeState[] = "vkb-change-state";
const char kVKBOverlap[] = "vkb-overlap";
}  // namespace events

gin::WrapperInfo BrowserShellWindow::kWrapperInfo = {gin::kEmbedderNativeGin};

char BrowserShellWindow::kPageViewPropertyName[] = "pageView";
char BrowserShellWindow::kSetVisibleMethodName[] = "setVisible";
char BrowserShellWindow::kIsVisibleMethodName[] = "isVisible";

BrowserShellWindow::BrowserShellWindow(
    v8::Isolate* isolate,
    mojo::Remote<browser_shell::mojom::ShellWindow> remote)
    : remote_(std::move(remote)), receiver_(this) {
  remote_->RegisterClient(receiver_.BindNewEndpointAndPassRemote());
  mojo::Remote<browser_shell::mojom::PageView> remote_view;
  auto view_pending_receiver = remote_view.BindNewPipeAndPassReceiver();

  BrowserShellPageView::CreateParams page_view_params;
  page_view_params.is_main_view = true;
  auto* shell_page_view = new injections::BrowserShellPageView(
      isolate, std::move(remote_view), page_view_params);

  remote_->BindPageView(std::move(view_pending_receiver),
                        base::BindOnce(&BrowserShellPageView::Setup,
                                       base::Unretained(shell_page_view)));

  gin::Handle<injections::BrowserShellPageView> handle =
      gin::CreateHandle(isolate, shell_page_view);

  if (!handle.IsEmpty()) {
    page_view_object_.Reset(isolate,
                            handle->GetWrapper(isolate).ToLocalChecked());
  }
}

BrowserShellWindow::~BrowserShellWindow() = default;

void BrowserShellWindow::Setup(const std::string& name) {
  name_ = name;
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

void BrowserShellWindow::Updated() {
  NOTIMPLEMENTED();
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

std::string& BrowserShellWindow::GetName() {
  if (name_.empty() && remote_.is_connected())
    remote_->SyncName(&name_);
  return name_;
}

v8::Local<v8::Object> BrowserShellWindow::GetPageView(v8::Isolate* isolate) {
  return page_view_object_.Get(isolate);
}

void BrowserShellWindow::SetVisible(bool visible) {
  NOTIMPLEMENTED();
}

bool BrowserShellWindow::IsVisible() const {
  NOTIMPLEMENTED();
  return true;
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
      .SetMethod(kSetVisibleMethodName, &BrowserShellWindow::SetVisible)
      .SetMethod(kIsVisibleMethodName, &BrowserShellWindow::IsVisible);
}

}  // namespace injections
