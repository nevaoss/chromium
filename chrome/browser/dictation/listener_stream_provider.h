// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_DICTATION_LISTENER_STREAM_PROVIDER_H_
#define CHROME_BROWSER_DICTATION_LISTENER_STREAM_PROVIDER_H_

#include <memory>
#include <string>

#include "base/functional/callback.h"
#include "base/memory/raw_ptr.h"
#include "chrome/browser/dictation/dictation_multiplexer.h"
#include "chrome/browser/dictation/stream_provider.h"

namespace content {
class BrowserContext;
}

namespace dictation {

class Target;

// A StreamProvider implementation that listens to dictation input via the
// dictation private extension API.
class ListenerStreamProvider : public StreamProvider {
 public:
  explicit ListenerStreamProvider(content::BrowserContext* browser_context);
  ~ListenerStreamProvider() override;

  ListenerStreamProvider(const ListenerStreamProvider&) = delete;
  ListenerStreamProvider& operator=(const ListenerStreamProvider&) = delete;

  // StreamProvider:
  void BindToTargetAndConnect(std::unique_ptr<Target> target) override;
  void Stop() override;
  void OnTranscriptionUpdated(const std::string& data, bool is_final) override;
  void OnStreamStateChanged(StreamState state) override;

  void SetOnUpdateForTesting(base::RepeatingClosure callback);
  const std::string& GetLatestTranscriptionForTesting() const;
  bool IsTranscriptionFinalForTesting() const;
  StreamState GetStateForTesting() const;

 private:
  DictationMultiplexer& GetMultiplexer() const;

  std::unique_ptr<Target> target_;
  raw_ptr<content::BrowserContext> browser_context_;
  bool needs_end_stream_ = false;
  DictationMultiplexer::StreamId stream_id_;
  std::string latest_transcription_;
  bool is_final_ = false;
  StreamState state_ = StreamState::kInitializing;

  base::RepeatingClosure update_callback_for_testing_;
};

}  // namespace dictation

#endif  // CHROME_BROWSER_DICTATION_LISTENER_STREAM_PROVIDER_H_
