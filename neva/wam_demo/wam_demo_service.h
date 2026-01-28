// Copyright 2017 LG Electronics, Inc.
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

#ifndef NEVA_WAM_DEMO_WAM_DEMO_SERVICE_H_
#define NEVA_WAM_DEMO_WAM_DEMO_SERVICE_H_

#include <optional>

#include "neva/emulator/emulator_data_source.h"
#include "neva/wam_demo/wam_demo_manager.h"
#include "neva/wam_demo/wam_demo_manager_delegate.h"

namespace wam_demo {

class WamDemoService : private emulator::EmulatorDataDelegate,
                       private WamDemoManagerDelegate {

 public:
  explicit WamDemoService(content::MainFunctionParams parameters);
  WamDemoService(const WamDemoService&) = delete;
  WamDemoService& operator=(const WamDemoService&) = delete;
  virtual ~WamDemoService() = default;

  void LaunchApplicationFromCLI(const std::string& appid,
                                const std::string& appurl,
                                bool fullscreen,
                                bool frameless);

 private:
  void HandleLaunchApplicationCommand(const std::string& value,
                                      const std::string& appid,
                                      const std::string& appurl);
  void EmulatorSendData(const std::string& command, const std::string& id);
  void PerformanceEvent(base::TimeDelta delta,
                        const std::string& appid,
                        const std::string& type);

  // WebAppManagerDelegate
  void AppWindowClosing(const std::string& appid) override;

  void CursorVisibilityChanged(const std::string& appid,
                               bool shown) override;

  void DidLoadingEnd(const std::string& appid,
                     base::TimeDelta delta) override;

  void DidFirstPaint(const std::string& appid,
                     base::TimeDelta delta) override;

  void DidFirstContentfulPaint(const std::string& appid,
                               base::TimeDelta delta) override;

  void DidFirstImagePaint(const std::string& appid,
                          base::TimeDelta delta) override;

  void DidFirstMeaningfulPaint(const std::string& appid,
                               base::TimeDelta delta) override;

  void DidLargestContentfulPaint(const std::string& appid,
                                 base::TimeDelta delta) override;

  void RenderProcessGone(const std::string& appid) override;

  void RenderProcessCreated(const std::string& appid) override;

  void DocumentLoadFinished(const std::string& appid,
                            std::optional<pid_t> pid,
                            float zoom) override;

  void LoadFailed(const std::string& appid) override;

  void BrowserControlCommandNotify(
      const std::string& name,
      const std::vector<std::string>& args) override;

  void BrowserControlFunctionNotify(
      const std::string& name,
      const std::vector<std::string>& args,
      const std::string& result) override;

  // from EmulatorDataDelegate
  void DataUpdated(const std::string& url, const std::string& data) override;

  const content::MainFunctionParams parameters_;
  WamDemoManager manager_;
  std::string profile_name_;
};

}  // namespace wam_demo

#endif  // NEVA_WAM_DEMO_WAM_DEMO_SERVICE_H_
