// Copyright 2022 LG Electronics, Inc.
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

#ifndef NEVA_APP_RUNTIME_PUBLIC_PERMISSION_PROMPT_H_
#define NEVA_APP_RUNTIME_PUBLIC_PERMISSION_PROMPT_H_

#include <string>
#include <vector>

namespace neva_app_runtime {

enum class PermissionSetting { kAllow, kBlock, kDefault };

class PermissionRequest {
 public:
  enum class RequestType {
    kInvalid,
    kCameraStream,
    kMicStream,
    kNotifications,
    kMaxValue = kNotifications
  };
  virtual ~PermissionRequest() {}

  virtual const std::string& RequestingOrigin() const = 0;
  virtual RequestType GetRequestType() const = 0;

  // Called when the user has granted the requested permission.
  // If |is_one_time| is true the permission will last until all tabs of
  // |origin| are closed or navigated away from, and then the permission will
  // automatically expire after 1 day.
  virtual void PermissionGranted(bool is_one_time) = 0;

  // Called when the user has denied the requested permission.
  virtual void PermissionDenied() = 0;

  // Called when the user has cancelled the permission request. This
  // corresponds to a denial, but is segregated in case the context needs to
  // be able to distinguish between an active refusal or an implicit refusal.
  virtual void Cancelled() = 0;

  // The UI this request was associated with was answered by the user.
  // It is safe for the request to be deleted at this point -- it will receive
  // no further message from the permission request system. This method will
  // eventually be called on every request which is not unregistered.
  // It is ok to call this method without actually resolving the request via
  // PermissionGranted(), PermissionDenied() or Canceled(). However, it will not
  // resolve the javascript promise from the requesting origin.
  virtual void RequestFinished() = 0;
};

class PermissionPrompt {
 public:
  class Delegate {
   public:
    virtual ~Delegate() {}
    virtual const std::vector<PermissionRequest*>& Requests() = 0;
    virtual const std::string& GetAppId() const = 0;
    virtual void Accept() = 0;
    virtual void AcceptThisTime() = 0;
    virtual void Deny() = 0;
    virtual void Closing() = 0;
  };
  virtual ~PermissionPrompt() {}
};

}  // namespace neva_app_runtime

#endif  // NEVA_APP_RUNTIME_PUBLIC_PERMISSION_PROMPT_H_
