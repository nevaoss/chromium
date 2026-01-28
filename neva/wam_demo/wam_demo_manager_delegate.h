// Copyright 2020 LG Electronics, Inc.
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

#ifndef NEVA_WAM_DEMO_WAM_DEMO_MANAGER_DELEGATE_H_
#define NEVA_WAM_DEMO_WAM_DEMO_MANAGER_DELEGATE_H_

#include <optional>
#include <string>
#include <vector>

#include "base/time/time.h"

namespace wam_demo {

class WamDemoManagerDelegate {
 public:
  virtual void AppWindowClosing(const std::string& appid) = 0;

  virtual void CursorVisibilityChanged(const std::string& appid,
                                       bool shown) = 0;

  virtual void DidLoadingEnd(const std::string& appid,
                             base::TimeDelta delta) = 0;

  virtual void DidFirstPaint(const std::string& appid,
                             base::TimeDelta delta) = 0;

  virtual void DidFirstContentfulPaint(const std::string& appid,
                                       base::TimeDelta delta) = 0;

  virtual void DidFirstImagePaint(const std::string& appid,
                                  base::TimeDelta delta) = 0;

  virtual void DidFirstMeaningfulPaint(const std::string& appid,
                                       base::TimeDelta delta) = 0;

  virtual void DidLargestContentfulPaint(const std::string& appid,
                                         base::TimeDelta delta) = 0;

  virtual void RenderProcessGone(const std::string& appid) = 0;

  virtual void RenderProcessCreated(const std::string& appid) = 0;

  virtual void DocumentLoadFinished(const std::string& appid,
                                    std::optional<pid_t> pid,
                                    float zoom) = 0;

  virtual void LoadFailed(const std::string& appid) = 0;

  virtual void BrowserControlCommandNotify(
      const std::string& name,
      const std::vector<std::string>& args) = 0;

  virtual void BrowserControlFunctionNotify(
      const std::string& name,
      const std::vector<std::string>& args,
      const std::string& result) = 0;
};

}  // namespace wam_demo

#endif  // NEVA_WAM_DEMO_WAM_DEMO_MANAGER_DELEGATE_H_
