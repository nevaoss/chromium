// Copyright 2016-2018 LG Electronics, Inc.
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

#ifndef NEVA_EMULATOR_EMULATOR_DATA_SOURCE_H_
#define NEVA_EMULATOR_EMULATOR_DATA_SOURCE_H_

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <set>
#include <vector>

#include "base/memory/weak_ptr.h"
#include "base/threading/thread.h"
#include "base/values.h"
#include "neva/emulator/emulator_export.h"
#include "url/gurl.h"

namespace base {
class TaskRunner;
}  // namespace base

namespace net {
class URLRequestContext;
}  // namespace net

namespace network {
class SharedURLLoaderFactory;
class SimpleURLLoader;
class TransitionalURLLoaderFactoryOwner;
}  // namespace network

namespace emulator {

// Forward declared so that deprecated URLFetcher class can friend it.
class EmulatorDataSource;

// Delegate interface for EmulatorDataSource consumers

class EmulatorDataDelegate {
 public:
  virtual void DataUpdated(const std::string& url, const std::string& data) = 0;
};

struct RequestArgumentDescription {
  const char *name;
  raw_ptr<const std::string> value;
};

struct ResponseArgumentDescription {
  const char *name;
  raw_ptr<std::string> value;
};

using RequestArgs = std::vector<RequestArgumentDescription>;
using ResponseArgs = std::vector<ResponseArgumentDescription>;

// This is a singleton which manages a single thread for HTTP requests for any
// stub data.

class EMULATOR_EXPORT EmulatorDataSource : public base::Thread {
 public:
  static EmulatorDataSource* GetInstance();
  ~EmulatorDataSource() override;

  // Set expectation on the Mock server. This call is asynchronous.
  // The implementation of the function is executed
  // on the EmulatorDataSource thread.
  static void SetExpectationAsync(const std::string& url,
                                  const std::string& arg);

  void AddURLForPolling(const std::string& url,
                        EmulatorDataDelegate* delegate,
                        const scoped_refptr<base::TaskRunner>& taskRunner);
  std::string GetCachedValueForURL(const std::string& url);
  static std::string PrepareRequestParams(RequestArgs& args);
  static std::string PrepareRequestParams(base::Value::Dict& request);
  static bool GetResponseParams(ResponseArgs& args,
                                const std::string& response);

  void OnURLLoadComplete(const network::SimpleURLLoader* source,
                         std::unique_ptr<std::string> response_body);

  // Gets a WeakPtr to this instance.
  base::WeakPtr<EmulatorDataSource> GetWeakPtr();

 private:
  EmulatorDataSource();
  void PeriodicPoll();
  void SetExpectation(const std::string url, const std::string arg);
  void LoadURLOnce(const std::string& url);

  // from base::Thread()
  void Init() override;

  static std::unique_ptr<EmulatorDataSource> instance_;
  static const int kPollingIntervalMs = 100;
  static std::string kEmulatorBaseURL;
  static GURL kExpectationURL;

  struct URLData;
  std::map<std::string, URLData> urls_;
  std::mutex mutex_;
  std::unique_ptr<net::URLRequestContext> url_request_context_;
  std::unique_ptr<network::TransitionalURLLoaderFactoryOwner>
      url_loader_factory_owner_;
  scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory_;
  std::set<std::unique_ptr<const network::SimpleURLLoader>> url_loaders_;
  std::set<std::unique_ptr<const network::SimpleURLLoader>>
      expectation_url_loaders_;
  base::WeakPtrFactory<EmulatorDataSource> weak_ptr_factory_;
};

}  // namespace emulator

#endif  // NEVA_EMULATOR_EMULATOR_DATA_SOURCE_H_
