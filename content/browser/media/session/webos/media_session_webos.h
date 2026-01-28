// Copyright 2021 LG Electronics, Inc.
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

#ifndef CONTENT_BROWSER_MEDIA_SESSION_WEBOS_MEDIA_SESSION_WEBOS_H_
#define CONTENT_BROWSER_MEDIA_SESSION_WEBOS_MEDIA_SESSION_WEBOS_H_

#include <string>

#include "base/neva/webos/luna_service_client.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "services/media_session/public/mojom/media_session.mojom.h"

namespace base {
class UnguessableToken;
}  // namespace base

namespace content {

class MediaSessionImpl;
class WebContentsImpl;

// MCS stands for Media Control Service. Media Control Service is a webos
// service that communicates with media session in web engine to get media
// information or provide media control functions such as play/pause/rewind/etc.

// This class is interlayer between native MediaSession and MCS
// MediaSession. This class is owned by the native MediaSession and will
// unregister MCS MediaSession when the native MediaSession is destroyed.
class MediaSessionWebOS final
    : public media_session::mojom::MediaSessionObserver {
 public:
  explicit MediaSessionWebOS(MediaSessionImpl* session);
  MediaSessionWebOS(const MediaSessionWebOS&) = delete;
  MediaSessionWebOS& operator=(const MediaSessionWebOS&) = delete;
  ~MediaSessionWebOS() override;

  // media_session::mojom::MediaSessionObserver implementation:
  void MediaSessionInfoChanged(
      media_session::mojom::MediaSessionInfoPtr session_info) override;
  void MediaSessionMetadataChanged(
      const std::optional<media_session::MediaMetadata>& metadata) override;
  void MediaSessionPositionChanged(
      const std::optional<media_session::MediaPosition>& position) override;
  void MediaSessionActionsChanged(
      const std::vector<media_session::mojom::MediaSessionAction>& action)
      override {}
  void MediaSessionImagesChanged(
      const base::flat_map<media_session::mojom::MediaSessionImageType,
                           std::vector<media_session::MediaImage>>& images)
      override {}

 private:
  enum class MediaKeyEvent {
    kUnsupported = 0,
    kPlay,
    kPause,
    kNext,
    kPrevious,
  };

  enum class PlaybackState {
    kUnknown = 0,
    kPaused,
    kPlaying,
    kStopped,
  };

  void RequestMediaSession(const std::string& request_id);
  bool RegisterMediaSession(const std::string& session_id);
  void UnregisterMediaSession();
  bool ActivateMediaSession(const std::string& session_id);
  void DeactivateMediaSession();
  void SetPlaybackStatus(PlaybackState playback_state);

  // Sets a value on the Metadata property and sends to MCS if necessary.
  void SetMetadataProperty(const std::string& property,
                           const std::string& value);

  // Receives the response from MCS and takes necessary action.
  void ReceiveMediaKeyEvent(const std::string& payload);

  void CheckReplyStatusMessage(const std::string& message);
  void HandleMediaKeyEvent(const std::string* key_event);
  void SetMediaTitle(const std::u16string& value);
  void SetMediaArtist(const std::u16string& value);
  void SetMediaAlbum(const std::u16string& value);

  MediaSessionWebOS::PlaybackState ConvertIntoWebOSPlaybackState(
      media_session::mojom::MediaPlaybackState mojom_state);

  // True if registration requested to com.webos.service.mediacontroller
  // service.
  bool registration_requested_ = false;
  bool mcs_call_denied_ = false;

  MediaSessionImpl* const media_session_;

  std::string application_id_;
  std::string session_id_;

  LSMessageToken subscribe_key_ = 0;
  std::unique_ptr<base::LunaServiceClient> luna_service_client_;

  media_session::mojom::MediaSessionInfo_SessionState state_ =
      media_session::mojom::MediaSessionInfo_SessionState::kInactive;

  MediaSessionWebOS::PlaybackState playback_state_ = PlaybackState::kStopped;
  base::TimeDelta duration_;

  mojo::Receiver<media_session::mojom::MediaSessionObserver> observer_receiver_{
      this};

  base::WeakPtr<MediaSessionWebOS> weak_this_;
  base::WeakPtrFactory<MediaSessionWebOS> weak_factory_{this};
};

}  // namespace content

#endif  // CONTENT_BROWSER_MEDIA_SESSION_WEBOS_MEDIA_SESSION_WEBOS_H_
