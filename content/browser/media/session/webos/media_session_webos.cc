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

#include "content/browser/media/session/webos/media_session_webos.h"

#include <optional>
#include <sstream>

#include "base/functional/bind.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/process/process.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task/bind_post_task.h"
#include "base/time/time.h"
#include "base/values.h"
#include "content/browser/media/session/media_session_impl.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/browser/media_session.h"
#include "neva/logging.h"
#include "services/media_session/public/cpp/media_position.h"
#include "services/media_session/public/mojom/audio_focus.mojom.h"
#include "third_party/blink/public/mojom/renderer_preferences.mojom.h"

namespace content {

namespace {

const char kAppId[] = "appId";
const char kMediaId[] = "mediaId";
const char kSubscribe[] = "subscribe";
const char kSubscribed[] = "subscribed";
const char kReturnValue[] = "returnValue";
const char kKeyEvent[] = "keyEvent";

const char kMediaMetaData[] = "mediaMetaData";
const char kMediaMetaDataTitle[] = "title";
const char kMediaMetaDataArtist[] = "artist";
const char kMediaMetaDataAlbum[] = "album";
const char kMediaMetaDataTotalDuration[] = "totalDuration";

const char kMediaPlayStatus[] = "playStatus";
const char kMediaPlayStatusStopped[] = "PLAYSTATE_STOPPED";
const char kMediaPlayStatusPaused[] = "PLAYSTATE_PAUSED";
const char kMediaPlayStatusPlaying[] = "PLAYSTATE_PLAYING";
const char kMediaPlayStatusNone[] = "PLAYSTATE_NONE";

const char kMediaPlayPosition[] = "playPosition";

const char kPlayEvent[] = "play";
const char kPauseEvent[] = "pause";
const char kNextEvent[] = "next";
const char kPreviousEvent[] = "previous";

const char kRegisterMediaSession[] = "registerMediaSession";
const char kUnregisterMediaSession[] = "unregisterMediaSession";
const char kActivateMediaSession[] = "activateMediaSession";
const char kDeactivateMediaSession[] = "deactivateMediaSession";
const char kSetMediaMetaData[] = "setMediaMetaData";
const char kSetMediaPlayStatus[] = "setMediaPlayStatus";
const char kSetMediaPlayPosition[] = "setMediaPlayPosition";

}  // namespace

#define BIND_POST_TASK_TO_CURRENT_LOOP(function) \
  (base::BindPostTaskToCurrentDefault(           \
      base::BindRepeating(function, weak_this_)))

MediaSessionWebOS::MediaSessionWebOS(MediaSessionImpl* session)
    : media_session_(session) {
  NEVA_DCHECK(media_session_);
  weak_this_ = weak_factory_.GetWeakPtr();

  content::WebContents* web_contents =
      static_cast<WebContentsImpl*>(session->web_contents());

  NEVA_DCHECK(web_contents);
  blink::RendererPreferences* renderer_prefs =
      web_contents->GetMutableRendererPrefs();

  NEVA_DCHECK(renderer_prefs);
  luna_service_client_.reset(
      new base::LunaServiceClient(renderer_prefs->application_id));

  application_id_ = renderer_prefs->application_id;

  NEVA_VLOGTF(0) << " Application id: " << application_id_;

  session->AddObserver(observer_receiver_.BindNewPipeAndPassRemote());
}

MediaSessionWebOS::~MediaSessionWebOS() {
  NEVA_VLOGTF(0);
  UnregisterMediaSession();
}

void MediaSessionWebOS::MediaSessionInfoChanged(
    media_session::mojom::MediaSessionInfoPtr session_info) {
  NEVA_VLOGTF(1) << " state: " << session_info->state
                 << ", playback_state: " << session_info->playback_state
                 << ", session_id_: " << session_id_
                 << ", request_id: " << media_session_->GetRequestId()
                 << ", registration_requested_: " << registration_requested_
                 << ", mcs_call_denied_: " << mcs_call_denied_;
  if (mcs_call_denied_)
    return;

  if (!registration_requested_ &&
      media_session::mojom::MediaSessionInfo_SessionState::kActive ==
          session_info->state) {
    NEVA_LOGF(INFO) << " state: active";
    RequestMediaSession(media_session_->GetRequestId().ToString());
    return;
  }

  SetPlaybackStatus(
      ConvertIntoWebOSPlaybackState(session_info->playback_state));
}

void MediaSessionWebOS::MediaSessionMetadataChanged(
    const std::optional<media_session::MediaMetadata>& metadata) {
  NEVA_VLOGTF(1);
  if (mcs_call_denied_)
    return;

  if (!metadata.has_value()) {
    NEVA_LOGF(ERROR) << " Metadata is not received";
    return;
  }

  SetMediaAlbum(metadata->title);
  SetMediaArtist(metadata->artist);
  SetMediaTitle(metadata->album);
}

void MediaSessionWebOS::SetMediaAlbum(const std::u16string& value) {
  if (!value.empty())
    SetMetadataProperty(kMediaMetaDataAlbum, base::UTF16ToUTF8(value));
}

void MediaSessionWebOS::SetMediaArtist(const std::u16string& value) {
  if (!value.empty())
    SetMetadataProperty(kMediaMetaDataArtist, base::UTF16ToUTF8(value));
}

void MediaSessionWebOS::SetMediaTitle(const std::u16string& value) {
  if (!value.empty())
    SetMetadataProperty(kMediaMetaDataTitle, base::UTF16ToUTF8(value));
}

void MediaSessionWebOS::MediaSessionPositionChanged(
    const std::optional<media_session::MediaPosition>& position) {
  if (mcs_call_denied_)
    return;

  if (!position.has_value()) {
    NEVA_LOGF(ERROR) << " position is not received";
    return;
  }

  NEVA_VLOGTF(3);

  base::Value::Dict root_dict;
  root_dict.Set(kMediaId, session_id_);
  root_dict.Set(kMediaPlayPosition,
                std::to_string(position->GetPosition().InSecondsF()));

  std::string payload;
  if (!base::JSONWriter::Write(root_dict, &payload)) {
    NEVA_LOGF(ERROR) << " Failed to write setMediaPlayPosition payload";
    return;
  }

  NEVA_VLOGTF(1) << " payload: " << payload;
  luna_service_client_->CallAsync(
      base::LunaServiceClient::GetServiceURI(
          base::LunaServiceClient::URIType::MEDIACONTROLLER,
          kSetMediaPlayPosition),
      payload,
      BIND_POST_TASK_TO_CURRENT_LOOP(
          &MediaSessionWebOS::CheckReplyStatusMessage));

  // Send duration
  base::TimeDelta new_duration = position->duration();
  if (duration_ == new_duration)
    return;

  duration_ = new_duration;
  SetMetadataProperty(kMediaMetaDataTotalDuration,
                      std::to_string(position->duration().InSecondsF()));
}

bool MediaSessionWebOS::RegisterMediaSession(const std::string& session_id) {
  if (session_id.empty()) {
    NEVA_LOGF(ERROR) << " Invalid session id";
    return false;
  }

  base::Value::Dict root_dict;
  root_dict.Set(kMediaId, session_id);
  root_dict.Set(kAppId, application_id_);
  root_dict.Set(kSubscribe, true);

  std::string payload;
  if (!base::JSONWriter::Write(root_dict, &payload)) {
    NEVA_LOGF(ERROR) << " Failed to write registMediaSession payload";
    return false;
  }

  NEVA_VLOGTF(1) << " payload: " << payload;
  luna_service_client_->Subscribe(
      base::LunaServiceClient::GetServiceURI(
          base::LunaServiceClient::URIType::MEDIACONTROLLER,
          kRegisterMediaSession),
      payload, &subscribe_key_,
      BIND_POST_TASK_TO_CURRENT_LOOP(&MediaSessionWebOS::ReceiveMediaKeyEvent));

  registration_requested_ = true;
  return true;
}

void MediaSessionWebOS::UnregisterMediaSession() {
  if (!registration_requested_) {
    NEVA_LOGF(ERROR) << " Session is already unregistered";
    return;
  }

  if (session_id_.empty()) {
    NEVA_LOGF(ERROR) << " No registered session";
    return;
  }

  if (playback_state_ != PlaybackState::kStopped)
    SetPlaybackStatus(PlaybackState::kStopped);

  base::Value::Dict root_dict;
  root_dict.Set(kMediaId, session_id_);

  std::string payload;
  if (!base::JSONWriter::Write(root_dict, &payload)) {
    NEVA_LOGF(ERROR) << " Failed to write unregistMediaSession payload";
    return;
  }

  NEVA_VLOGTF(1) << " payload: " << payload;
  luna_service_client_->CallAsync(
      base::LunaServiceClient::GetServiceURI(
          base::LunaServiceClient::URIType::MEDIACONTROLLER,
          kUnregisterMediaSession),
      payload,
      BIND_POST_TASK_TO_CURRENT_LOOP(
          &MediaSessionWebOS::CheckReplyStatusMessage));

  luna_service_client_->Unsubscribe(subscribe_key_);

  registration_requested_ = false;
  session_id_ = std::string();
  duration_ = base::TimeDelta();
}

bool MediaSessionWebOS::ActivateMediaSession(const std::string& session_id) {
  if (session_id.empty()) {
    NEVA_LOGF(ERROR) << " Invalid session id";
    return false;
  }

  base::Value::Dict root_dict;
  root_dict.Set(kMediaId, session_id);

  std::string payload;
  if (!base::JSONWriter::Write(root_dict, &payload)) {
    NEVA_LOGF(ERROR) << " Failed to write activateMediaSession payload";
    return false;
  }

  NEVA_VLOGTF(1) << " payload: " << payload;
  luna_service_client_->CallAsync(
      base::LunaServiceClient::GetServiceURI(
          base::LunaServiceClient::URIType::MEDIACONTROLLER,
          kActivateMediaSession),
      payload,
      BIND_POST_TASK_TO_CURRENT_LOOP(
          &MediaSessionWebOS::CheckReplyStatusMessage));

  return true;
}

void MediaSessionWebOS::DeactivateMediaSession() {
  if (session_id_.empty()) {
    NEVA_LOGF(ERROR) << " No active session";
    return;
  }

  base::Value::Dict root_dict;
  root_dict.Set(kMediaId, session_id_);

  std::string payload;
  if (!base::JSONWriter::Write(root_dict, &payload)) {
    NEVA_LOGF(ERROR) << " Failed to write deactivateMediaSession payload";
    return;
  }

  NEVA_VLOGTF(1) << " payload: " << payload;
  luna_service_client_->CallAsync(
      base::LunaServiceClient::GetServiceURI(
          base::LunaServiceClient::URIType::MEDIACONTROLLER,
          kDeactivateMediaSession),
      payload,
      BIND_POST_TASK_TO_CURRENT_LOOP(
          &MediaSessionWebOS::CheckReplyStatusMessage));
}

void MediaSessionWebOS::SetPlaybackStatus(PlaybackState playback_state) {
  if (session_id_.empty()) {
    NEVA_LOGF(ERROR) << " No active session";
    return;
  }

  if (playback_state_ == playback_state)
    return;

  static std::map<PlaybackState, std::string> kPlaybackStateMap = {
      {PlaybackState::kPaused, kMediaPlayStatusPaused},
      {PlaybackState::kPlaying, kMediaPlayStatusPlaying},
      {PlaybackState::kStopped, kMediaPlayStatusStopped}};

  auto get_playback_status = [&](PlaybackState status) {
    auto it = kPlaybackStateMap.find(status);
    if (it != kPlaybackStateMap.end())
      return it->second;
    return std::string(kMediaPlayStatusNone);
  };

  playback_state_ = playback_state;
  std::string play_status = get_playback_status(playback_state_);

  base::Value::Dict root_dict;
  root_dict.Set(kMediaId, session_id_);
  root_dict.Set(kMediaPlayStatus, play_status);

  std::string payload;
  if (!base::JSONWriter::Write(root_dict, &payload)) {
    NEVA_LOGF(ERROR) << " Failed to write setMediaPlayStatus payload";
    return;
  }

  NEVA_VLOGTF(1) << " payload: " << payload;
  luna_service_client_->CallAsync(
      base::LunaServiceClient::GetServiceURI(
          base::LunaServiceClient::URIType::MEDIACONTROLLER,
          kSetMediaPlayStatus),
      payload,
      BIND_POST_TASK_TO_CURRENT_LOOP(
          &MediaSessionWebOS::CheckReplyStatusMessage));
}

void MediaSessionWebOS::SetMetadataProperty(const std::string& property,
                                            const std::string& value) {
  base::Value::Dict metadata;
  metadata.Set(property, value);

  base::Value::Dict root_dict;
  root_dict.Set(kMediaId, session_id_);
  root_dict.Set(kMediaMetaData, std::move(metadata));

  std::string payload;
  if (!base::JSONWriter::Write(root_dict, &payload)) {
    NEVA_LOGF(ERROR) << " Failed to write setMediaMetaData payload";
    return;
  }

  NEVA_VLOGTF(1) << " payload: " << payload;
  luna_service_client_->CallAsync(
      base::LunaServiceClient::GetServiceURI(
          base::LunaServiceClient::URIType::MEDIACONTROLLER, kSetMediaMetaData),
      payload,
      BIND_POST_TASK_TO_CURRENT_LOOP(
          &MediaSessionWebOS::CheckReplyStatusMessage));
}

void MediaSessionWebOS::ReceiveMediaKeyEvent(const std::string& payload) {
  NEVA_VLOGTF(1) << " payload: " << payload;
  std::optional<base::Value> value = base::JSONReader::Read(payload);
  if (!value || !value->is_dict()) {
    return;
  }
  auto response =
      std::make_unique<base::Value::Dict>(std::move(value->GetDict()));
  auto return_value = response->FindBoolByDottedPath(kReturnValue);
  auto subscribed = response->FindBoolByDottedPath(kSubscribed);

  if (!return_value || !*return_value || !subscribed || !*subscribed) {
    mcs_call_denied_ = true;
    NEVA_LOGF(ERROR) << " Failed to Register with MCS"
                     << ", session_id: " << session_id_
                     << ", mcs_call_denied: " << mcs_call_denied_;
    return;
  }

  const std::string* session_id = response->FindStringByDottedPath(kMediaId);

  if (!session_id || session_id->empty()) {
    NEVA_LOGF(WARNING) << " Session ID is not sent";
    return;
  }

  if (session_id_ != *session_id) {
    NEVA_VLOGTF(1) << " Event recieved for other session. Ignore.";
    return;
  }

  const std::string* key_event = response->FindStringByDottedPath(kKeyEvent);
  if (key_event) {
    HandleMediaKeyEvent(key_event);
  } else {
    NEVA_VLOGTF(0) << " Successfully Registered with MCS, session_id: "
                   << session_id_;
  }
}

void MediaSessionWebOS::CheckReplyStatusMessage(const std::string& message) {
  NEVA_VLOGTF(1) << " message: " << message;
  std::optional<base::Value> value = base::JSONReader::Read(message);
  if (!value || !value->is_dict()) {
    return;
  }

  auto response =
      std::make_unique<base::Value::Dict>(std::move(value->GetDict()));
  auto return_value = response->FindBoolByDottedPath(kReturnValue);
  if (!return_value || !*return_value) {
    NEVA_LOGF(ERROR) << " MCS call Failed. message: " << message
                     << " session_id: " << session_id_;
    return;
  }

  NEVA_VLOGTF(1) << " MCS call Success. message: " << message
                 << " session_id: " << session_id_;
}

void MediaSessionWebOS::HandleMediaKeyEvent(const std::string* key_event) {
  NEVA_VLOGTF(0) << " key_event: " << *key_event;

  static std::map<std::string, MediaKeyEvent> kEventKeyMap = {
      {kPlayEvent, MediaSessionWebOS::MediaKeyEvent::kPlay},
      {kPauseEvent, MediaSessionWebOS::MediaKeyEvent::kPause},
      {kNextEvent, MediaSessionWebOS::MediaKeyEvent::kNext},
      {kPreviousEvent, MediaSessionWebOS::MediaKeyEvent::kPrevious}};

  auto get_event_type = [&](const std::string* key) {
    std::map<std::string, MediaKeyEvent>::iterator it;
    it = kEventKeyMap.find(*key);
    if (it != kEventKeyMap.end())
      return it->second;
    return MediaSessionWebOS::MediaKeyEvent::kUnsupported;
  };

  if (media_session_) {
    MediaKeyEvent event_type = get_event_type(key_event);
    switch (event_type) {
      case MediaSessionWebOS::MediaKeyEvent::kPlay:
        media_session_->Resume(MediaSession::SuspendType::kUI);
        break;
      case MediaSessionWebOS::MediaKeyEvent::kPause:
        media_session_->Suspend(MediaSession::SuspendType::kUI);
        break;
      case MediaSessionWebOS::MediaKeyEvent::kNext:
        media_session_->NextTrack();
        break;
      case MediaSessionWebOS::MediaKeyEvent::kPrevious:
        media_session_->PreviousTrack();
        break;
      default:
        NOTREACHED() << " key_event: " << key_event << " Not Handled !!!";
        break;
    }
  }
}

void MediaSessionWebOS::RequestMediaSession(const std::string& request_id) {
  if (!session_id_.empty()) {
    // Previous session is active. Deactivate it.
    UnregisterMediaSession();
  }

  if (request_id.empty()) {
    NEVA_LOGF(ERROR) << " Session id is not received";
    return;
  }

  if (!RegisterMediaSession(request_id)) {
    NEVA_LOGF(ERROR) << " Register session failed for " << request_id;
    return;
  }

  if (!ActivateMediaSession(request_id)) {
    NEVA_LOGF(ERROR) << " Activate session failed for " << request_id;
    return;
  }

  session_id_ = request_id;
}

MediaSessionWebOS::PlaybackState
MediaSessionWebOS::ConvertIntoWebOSPlaybackState(
    media_session::mojom::MediaPlaybackState mojom_state) {
  switch (mojom_state) {
    case media_session::mojom::MediaPlaybackState::kPaused:
      return PlaybackState::kPaused;
    case media_session::mojom::MediaPlaybackState::kPlaying:
      return PlaybackState::kPlaying;
    default:
      return PlaybackState::kUnknown;
  }
}

}  // namespace content
