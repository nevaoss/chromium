// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/readaloud/read_aloud_service.h"

#include "chrome/browser/profiles/profile.h"

namespace readaloud {

ReadAloudService::ReadAloudService(Profile* profile) : profile_(profile) {}

ReadAloudService::~ReadAloudService() = default;

}  // namespace readaloud
