// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_READALOUD_READ_ALOUD_SERVICE_H_
#define CHROME_BROWSER_READALOUD_READ_ALOUD_SERVICE_H_

#include "base/memory/raw_ptr.h"
#include "components/keyed_service/core/keyed_service.h"

class Profile;

namespace readaloud {

// Central lifecycle and state orchestrator for Read Aloud.
class ReadAloudService : public KeyedService {
 public:
  explicit ReadAloudService(Profile* profile);

  ReadAloudService(const ReadAloudService&) = delete;
  ReadAloudService& operator=(const ReadAloudService&) = delete;

  ~ReadAloudService() override;

 private:
  raw_ptr<Profile> profile_;
};

}  // namespace readaloud

#endif  // CHROME_BROWSER_READALOUD_READ_ALOUD_SERVICE_H_
