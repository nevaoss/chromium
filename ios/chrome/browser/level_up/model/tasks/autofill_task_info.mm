// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "base/functional/callback_helpers.h"
#import "base/strings/sys_string_conversions.h"
#import "ios/chrome/browser/level_up/model/task_info.h"
#import "ios/chrome/browser/level_up/model/tasks/task_factories.h"
#import "ios/chrome/browser/shared/ui/symbols/symbols.h"

class AutofillTaskInfo : public TaskInfo {
 public:
  AutofillTaskInfo() = default;
  ~AutofillTaskInfo() override = default;

  // TaskInfo implementation.
  TaskType GetTaskType() const override { return TaskType::kAutofill; }
  int GetTitleId() const override { return 0; }
  int GetTaskDescriptionId() const override { return 0; }
  std::string GetIconSymbolName() const override {
    return base::SysNSStringToUTF8(kPasswordManagerSymbol);
  }
  bool IsCustomSymbol() const override { return true; }
  LevelUpTaskCategory GetCategory() const override {
    return LevelUpTaskCategory::kProductivity;
  }
  std::string GetTriggerUserAction() const override { return ""; }
  base::RepeatingClosure GetNavigationAction() const override {
    return base::DoNothing();
  }
};

std::unique_ptr<TaskInfo> CreateAutofillTaskInfo() {
  return std::make_unique<AutofillTaskInfo>();
}
