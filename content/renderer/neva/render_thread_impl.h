// Copyright 2016-2018 LG Electronics, Inc.
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

#ifndef CONTENT_RENDERER_NEVA_RENDER_THREAD_IMPL_H_
#define CONTENT_RENDERER_NEVA_RENDER_THREAD_IMPL_H_

#if defined(USE_NEVA_SUSPEND_MEDIA_CAPTURE)
#include "content/renderer/media/audio/neva/audio_capturer_source_manager.h"
#endif

namespace content {
namespace neva {

template <typename original_t>
class RenderThreadImpl {
 public:
#if defined(USE_NEVA_SUSPEND_MEDIA_CAPTURE)
  AudioCapturerSourceManager* audio_capturer_source_manager() const {
    return ac_manager_.get();
  }
#endif

 protected:
  void Init();
#if defined(USE_NEVA_SUSPEND_MEDIA_CAPTURE)
  std::unique_ptr<AudioCapturerSourceManager> ac_manager_;
#endif
};

template <typename original_t>
void RenderThreadImpl<original_t>::Init() {
#if defined(USE_NEVA_SUSPEND_MEDIA_CAPTURE)
  ac_manager_.reset(new AudioCapturerSourceManager());
#endif
}

}  // namespace neva
}  // namespace content

#endif  // CONTENT_RENDERER_NEVA_RENDER_THREAD_IMPL_H_
