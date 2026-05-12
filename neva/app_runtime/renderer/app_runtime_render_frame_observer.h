// Copyright 2016-2019 LG Electronics, Inc.
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

#ifndef NEVA_APP_RUNTIME_RENDERER_APP_RUNTIME_RENDER_FRAME_OBSERVER_H_
#define NEVA_APP_RUNTIME_RENDERER_APP_RUNTIME_RENDER_FRAME_OBSERVER_H_

#include "content/public/renderer/render_frame_observer.h"
#include "mojo/public/cpp/bindings/associated_receiver.h"
#include "neva/app_runtime/public/mojom/app_runtime_webview.mojom.h"
#include "neva/app_runtime/renderer/app_runtime_injection_loader.h"

namespace neva_app_runtime {

class AppRuntimeRenderFrameObserver : public content::RenderFrameObserver,
                                      public mojom::AppRuntimeWebViewClient {
 public:
  AppRuntimeRenderFrameObserver(content::RenderFrame* render_frame);
  ~AppRuntimeRenderFrameObserver() override;

  // RenderFrameObserver
  void OnDestruct() override;
  void DidClearWindowObject() override;
  void ScriptedPrint(bool user_initiated) override;

  // mojom::AppRuntimeWebViewClient implementation.
  void SetVisibilityState(mojom::VisibilityState visibility_state) override;
  void ChangeLocale(const std::string& locale) override;
  void SetNetworkQuietTimeout(double timeout) override;
  void SetDisallowScrollbarsInMainFrame(bool disallow) override;
  void GrantLoadLocalResources() override;
  void InsertStyleSheet(const std::string& css) override;
  void ReplaceBaseURL(const std::string& new_url) override;
  void SetWebViewInfo(const std::string& app_path,
                      const std::string& trust_level) override;

  void AddInjectionToLoad(const std::string& injection,
                          const std::string& arguments_json) override;
  void UnloadInjections() override;

  void BindPendingReceiver(mojo::PendingAssociatedReceiver<
                           mojom::AppRuntimeWebViewClient> pending_receiver);

 private:
  // Whether DOM activity is suspended or not.
  bool dom_suspended_ = false;
  InjectionLoader injection_loader_;
  mojo::AssociatedReceiver<mojom::AppRuntimeWebViewClient> receiver_{this};
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_RENDERER_APP_RUNTIME_RENDER_FRAME_OBSERVER_H_
