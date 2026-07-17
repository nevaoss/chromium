// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/dictation/target.h"

#include <utility>

namespace dictation {

Target::Target() = default;

Target::Target(const std::string& selected_text)
    : selected_text_(selected_text) {}

Target::~Target() = default;

const std::string& Target::GetSelectedText() const {
  return selected_text_;
}

}  // namespace dictation
