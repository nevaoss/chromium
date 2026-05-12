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
#ifndef NEVA_INJECTION_RENDERER_BROWSER_SHELL_BROWSER_SHELL_PAGE_VIEW_H_
#define NEVA_INJECTION_RENDERER_BROWSER_SHELL_BROWSER_SHELL_PAGE_VIEW_H_

#include <map>

#include "base/memory/raw_ptr.h"
#include "gin/object_template_builder.h"
#include "gin/wrappable.h"
#include "mojo/public/cpp/bindings/associated_receiver.h"
#include "mojo/public/cpp/bindings/pending_associated_receiver.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "neva/browser_shell/service/public/mojom/browser_shell_page_view.mojom.h"
#include "neva/browser_shell/service/public/mojom/browser_shell_service.mojom.h"
#include "neva/injection/renderer/injection_events_emitter.h"
#include "ui/gfx/geometry/rect.h"
#include "v8/include/v8.h"

namespace gin {
class Arguments;
}

namespace injections {

class BrowserShellPageContents;

class BrowserShellPageView
    : public gin::Wrappable<BrowserShellPageView>,
      public InjectionEventsEmitter<BrowserShellPageView>,
      public browser_shell::mojom::PageViewClient {
 public:
  static gin::WrapperInfo kWrapperInfo;

  static char kIdPropertyName[];
  static char kPageContentsPropertyName[];

  static char kAddChildViewMethodName[];
  static char kBringToFrontMethodName[];
  static char kGetBoundsMethodName[];
  static char kHasParentViewMethodName[];
  static char kHasChildViewMethodName[];
  static char kGetChildViewsMethodName[];
  static char kIsVisibleMethodName[];
  static char kRemoveChildViewMethodName[];
  static char kSendToBackMethodName[];
  static char kSetBoundsMethodName[];
  static char kSetVisibleMethodName[];

  static void ConstructorCallback(
      mojo::Remote<browser_shell::mojom::ShellService>* shell_service,
      gin::Arguments* args);

  static v8::Local<v8::Object> BuildOptions(v8::Isolate* isolate,
                                            v8::Local<v8::Value> json_value);
  struct CreateParams {
    bool error_page_hiding = false;
    bool is_main_view = false;
  };

  BrowserShellPageView(v8::Isolate* isolate,
                       mojo::Remote<browser_shell::mojom::PageView>,
                       const CreateParams& params);
  BrowserShellPageView(const BrowserShellPageView&) = delete;
  BrowserShellPageView& operator=(const BrowserShellPageView&) = delete;
  ~BrowserShellPageView() override;

  uint64_t GetID();
  bool IsMainView() const;
  void Setup(uint64_t id);

  // browser_shell::mojom::PageViewClient
  void VisibilityChanged(bool visible, uint32_t reason_code) override;

  // ObjectTemplateBuilder::SetMethod does not support exposing inherited
  // methods. Such proxy-calling inherited methods is ugly but the easiest
  // workaround.
  void RunGetEventNames(gin::Arguments* args) const {
    InjectionEventsEmitter::GetEventNames(args);
  }
  void RunEmit(gin::Arguments* args) { InjectionEventsEmitter::Emit(args); }
  void RunAddEventListener(gin::Arguments* args) {
    InjectionEventsEmitter::AddEventListener(args);
  }
  void RunAddOnceEventListener(gin::Arguments* args) {
    InjectionEventsEmitter::AddOnceEventListener(args);
  }
  int RunGetListenerCount(const std::string& name) const {
    return InjectionEventsEmitter::GetListenerCount(name);
  }
  void RunRemoveEventListener(gin::Arguments* args) {
    InjectionEventsEmitter::RemoveEventListener(args);
  }
  void RunRemoveAllEventListeners(gin::Arguments* args) {
    InjectionEventsEmitter::RemoveAllEventListeners(args);
  }

 private:
  bool HasParentView() const;
  void SetParentView(BrowserShellPageView* parent_view);

  v8::Local<v8::Object> GetPageContents(v8::Isolate* isolate);
  void SetPageContents(v8::Isolate* isolate, v8::Local<v8::Object> object);

  bool AddChildView(v8::Isolate* isolate, v8::Local<v8::Object> object);
  void RemoveChildView(v8::Isolate* isolate, v8::Local<v8::Object> object);
  void GetChildViews(gin::Arguments* args);
  bool HasChildView(v8::Isolate* isolate, v8::Local<v8::Object> object) const;

  void SetVisible(bool visible);
  bool IsVisible() const;
  void SetBounds(int x, int y, int w, int h);
  void GetBounds(gin::Arguments* args) const;
  void BringToFront(gin::Arguments* args);
  void SendToBack(gin::Arguments* args);

  gin::ObjectTemplateBuilder GetObjectTemplateBuilder(
      v8::Isolate* isolate) override;

  bool is_main_view_;
  uint64_t id_ = 0;
  bool visible_ = true;
  raw_ptr<BrowserShellPageView> parent_view_ = nullptr;
  gfx::Rect bounds_;
  mojo::Remote<browser_shell::mojom::PageView> remote_;
  mojo::AssociatedReceiver<browser_shell::mojom::PageViewClient>
      receiver_;
  v8::Global<v8::Object> page_contents_object_;
  std::map<uint64_t, v8::Global<v8::Object>> child_view_objects_;
};

}  // namespace injections

#endif  // NEVA_INJECTION_RENDERER_BROWSER_SHELL_BROWSER_SHELL_PAGE_VIEW_H_
