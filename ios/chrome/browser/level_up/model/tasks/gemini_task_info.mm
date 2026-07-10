// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "base/functional/callback_helpers.h"
#import "base/strings/sys_string_conversions.h"
#import "ios/chrome/browser/level_up/model/task_info.h"
#import "ios/chrome/browser/level_up/model/tasks/task_factories.h"
#import "ios/chrome/browser/shared/ui/buildflags.h"
#import "ios/chrome/browser/shared/ui/symbols/symbols.h"

class GeminiTaskInfo : public TaskInfo {
 public:
  GeminiTaskInfo() = default;
  ~GeminiTaskInfo() override = default;

  // TaskInfo implementation.
  TaskType GetTaskType() const override { return TaskType::kGemini; }
  int GetTitleId() const override { return 0; }
  int GetTaskDescriptionId() const override { return 0; }
  std::string GetIconSymbolName() const override {
#if BUILDFLAG(IOS_USE_BRANDED_ASSETS)
    return base::SysNSStringToUTF8(kGeminiBrandedLogoSymbol);
#else
    return base::SysNSStringToUTF8(kGeminiNonBrandedLogoSymbol);
#endif
  }
  bool IsCustomSymbol() const override {
#if BUILDFLAG(IOS_USE_BRANDED_ASSETS)
    return true;
#else
    return false;
#endif
  }
  LevelUpTaskCategory GetCategory() const override {
    return LevelUpTaskCategory::kProductivity;
  }
  std::string GetTriggerUserAction() const override { return ""; }
  base::RepeatingClosure GetNavigationAction() const override {
    return base::DoNothing();
  }
};

std::unique_ptr<TaskInfo> CreateGeminiTaskInfo() {
  return std::make_unique<GeminiTaskInfo>();
}
