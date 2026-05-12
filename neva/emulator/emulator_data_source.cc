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

#include "emulator_data_source.h"

#include <algorithm>

#include "base/functional/bind.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/json/string_escape.h"
#include "base/memory/ref_counted.h"
#include "base/values.h"
#include "emulator_urls.h"
#include "net/http/http_status_code.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_request_context_builder.h"
#include "net/url_request/url_request_context_getter.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"
#include "services/network/public/cpp/simple_url_loader.h"
#include "services/network/public/mojom/url_response_head.mojom.h"
#include "services/network/transitional_url_loader_factory_owner.h"

#if defined(OS_LINUX)
#include "net/proxy_resolution/proxy_config.h"
#include "net/proxy_resolution/proxy_config_service_fixed.h"
#endif

namespace {

const char kUploadContentType[] = "text/plain; charset=utf-8";

// Builds a URLRequestContext assuming there's only a single loop.
std::unique_ptr<net::URLRequestContext> BuildURLRequestContext() {
  net::URLRequestContextBuilder builder;
#if defined(OS_LINUX)
  // On Linux, use a fixed ProxyConfigService, since the default one
  // depends on glib.
  //
  // TODO(akalin): Remove this once http://crbug.com/146421 is fixed.
  builder.set_proxy_config_service(
      std::make_unique<net::ProxyConfigServiceFixed>(
          net::ProxyConfigWithAnnotation()));
#endif
  std::unique_ptr<net::URLRequestContext> context(builder.Build());
  return context;
}

}  // namespace

// FIXME(neva): net::TrivialURLRequestContextGetter was removed in upstream
// https://chromium-review.googlesource.com/c/chromium/src/+/2176672
namespace net {
class TrivialURLRequestContextGetter : public URLRequestContextGetter {
 public:
  TrivialURLRequestContextGetter(
      URLRequestContext* context,
      const scoped_refptr<base::SingleThreadTaskRunner>& main_task_runner)
      : context_(context), main_task_runner_(main_task_runner) {}
  TrivialURLRequestContextGetter(const TrivialURLRequestContextGetter&) =
      delete;
  TrivialURLRequestContextGetter& operator=(
      const TrivialURLRequestContextGetter&) = delete;

  // URLRequestContextGetter implementation:
  URLRequestContext* GetURLRequestContext() override { return context_; }

  scoped_refptr<base::SingleThreadTaskRunner> GetNetworkTaskRunner()
      const override {
    return main_task_runner_;
  }

 private:
  ~TrivialURLRequestContextGetter() override = default;

  raw_ptr<URLRequestContext> context_;
  const scoped_refptr<base::SingleThreadTaskRunner> main_task_runner_;
};

}  // namespace net

namespace emulator {

struct EmulatorDataSource::URLData {
  URLData() {}
  ~URLData() {}
  raw_ptr<EmulatorDataDelegate> delegate = nullptr;
  std::string cached_data;
  scoped_refptr<base::TaskRunner> task_runner;
  std::string url;
};

std::unique_ptr<EmulatorDataSource> EmulatorDataSource::instance_;
std::string EmulatorDataSource::kEmulatorBaseURL;
GURL EmulatorDataSource::kExpectationURL;

EmulatorDataSource* EmulatorDataSource::GetInstance() {
  if (instance_ == nullptr) {
    char* neva_emulator_server_address =
        ::getenv("NEVA_EMULATOR_SERVER_ADDRESS");

    kEmulatorBaseURL = "http://" +
                       std::string((neva_emulator_server_address != nullptr)
                                       ? neva_emulator_server_address
                                       : kEmulatorDefaultHost) +
                       ":" + std::to_string(kEmulatorDefaultPort) + "/";

    kExpectationURL =
        GURL((kEmulatorBaseURL + kEmulatorExpectationPath).c_str());

    instance_.reset(new EmulatorDataSource());
  }

  return instance_.get();
}

EmulatorDataSource::EmulatorDataSource()
    : base::Thread("Emulator_DataFetcherThread"), weak_ptr_factory_(this) {
  Options options;
  options.message_pump_type = base::MessagePumpType::IO;
  StartWithOptions(std::move(options));
}

void EmulatorDataSource::Init() {
  url_request_context_ = BuildURLRequestContext();
  url_loader_factory_owner_ =
      std::make_unique<network::TransitionalURLLoaderFactoryOwner>(
          base::MakeRefCounted<net::TrivialURLRequestContextGetter>(
              url_request_context_.get(), task_runner()));
  url_loader_factory_ = url_loader_factory_owner_->GetURLLoaderFactory();

  // Start polling
  PeriodicPoll();
}

EmulatorDataSource::~EmulatorDataSource() {
  Stop();
}

void EmulatorDataSource::AddURLForPolling(
    const std::string& url,
    EmulatorDataDelegate* delegate,
    const scoped_refptr<base::TaskRunner>& taskRunner) {
  std::lock_guard<std::mutex> lock(mutex_);
  std::string fetch_url = kEmulatorBaseURL + url;
  URLData& url_data = urls_[fetch_url];
  url_data.delegate = delegate;
  url_data.task_runner = taskRunner;
  url_data.url = url;
}

std::string EmulatorDataSource::GetCachedValueForURL(const std::string& url) {
  std::lock_guard<std::mutex> lock(mutex_);
  std::string fetch_url = kEmulatorBaseURL + url;
  return urls_[fetch_url].cached_data;
}

void EmulatorDataSource::LoadURLOnce(const std::string& url) {
  auto resource_request = std::make_unique<network::ResourceRequest>();
  resource_request->url = GURL(url);
  resource_request->method = net::HttpRequestHeaders::kGetMethod;

  auto url_loader = network::SimpleURLLoader::Create(
      std::move(resource_request), MISSING_TRAFFIC_ANNOTATION);
  url_loader->SetAllowHttpErrorResults(true);
  auto* url_loader_ptr = url_loader.get();
  url_loader_ptr->DownloadToStringOfUnboundedSizeUntilCrashAndDie(
      url_loader_factory_.get(),
      base::BindOnce(&EmulatorDataSource::OnURLLoadComplete,
                     base::Unretained(this), url_loader_ptr));

  {
    std::lock_guard<std::mutex> lock(mutex_);
    url_loaders_.emplace(std::move(url_loader));
  }
}

void EmulatorDataSource::PeriodicPoll() {
  std::vector<std::string> urls;

  // Extracting URL values from the URL entries added for polling
  {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = urls_.begin(); it != urls_.end(); ++it)
      urls.push_back(it->first);
  }

  // Traversing the URL values just extracted, trying to fetch related data
  for (auto it = urls.begin(); it != urls.end(); ++it)
    LoadURLOnce(*it);

  // NOTE: the recursive call for polling below provides (most probably) one and
  // only correct alternative to polling via |base::Timer| instance while using
  // the Chromium |net| API set (otherwise, |base::Timer| and |net| library
  // become incompatible with each other)
  task_runner()->PostDelayedTask(
      FROM_HERE,
      base::BindOnce(&EmulatorDataSource::PeriodicPoll, GetWeakPtr()),
      base::Milliseconds(kPollingIntervalMs));
}

void EmulatorDataSource::SetExpectation(const std::string url,
                                        const std::string arg) {
  std::string request =
      R"JSON({"httpRequest":{"method":"","path":"/)JSON" + url +
      R"JSON(","queryStringParameters":[],"body":"","headers":[],"cookies":[]},
      )JSON";
  std::string response =
      R"JSON("httpResponse":{"statusCode":200,"body":")JSON" + arg +
      R"JSON(","cookies":[],"headers":[{"name":"Content-Type","values":
      ["text/plain;charset=utf-8"]},{"name":"Cache-Control","values":
      ["no-cache,no-store"]}],"delay":{"timeUnit":"MICROSECONDS","value":0}},
      )JSON";
  std::string options =
      R"JSON("times":{"remainingTimes":1,"unlimited":false}})JSON";

  auto resource_request = std::make_unique<network::ResourceRequest>();
  resource_request->url = GURL(kExpectationURL);
  resource_request->method = net::HttpRequestHeaders::kPutMethod;

  auto url_loader = network::SimpleURLLoader::Create(
      std::move(resource_request), MISSING_TRAFFIC_ANNOTATION);
  url_loader->SetAllowHttpErrorResults(true);
  url_loader->AttachStringForUpload(request + response + options,
                                    kUploadContentType);
  auto* url_loader_ptr = url_loader.get();
  url_loader_ptr->DownloadToStringOfUnboundedSizeUntilCrashAndDie(
      url_loader_factory_.get(),
      base::BindOnce(&EmulatorDataSource::OnURLLoadComplete,
                     base::Unretained(this), url_loader_ptr));

  {
    std::lock_guard<std::mutex> lock(mutex_);
    expectation_url_loaders_.emplace(std::move(url_loader));
  }
}

void EmulatorDataSource::SetExpectationAsync(const std::string& url,
                                             const std::string& arg) {
  EmulatorDataSource* instance = GetInstance();

  instance->task_runner()->PostTask(
      FROM_HERE, base::BindOnce(&EmulatorDataSource::SetExpectation,
                            instance->GetWeakPtr(), url, arg));
}

std::string EmulatorDataSource::PrepareRequestParams(RequestArgs &args) {
  base::Value::Dict request;
  for (auto arg: args) {
    request.Set(arg.name, *arg.value);
  }
  return PrepareRequestParams(request);
}

std::string EmulatorDataSource::PrepareRequestParams(
    base::Value::Dict& request) {
  std::string params;
  base::JSONWriter::Write(request, &params);
  std::string esc_params;
  base::EscapeJSONString(params, false, &esc_params);
  return esc_params;
}

bool EmulatorDataSource::GetResponseParams(ResponseArgs &args,
    const std::string& response) {
  auto ret = base::JSONReader::ReadAndReturnValueWithError(response);
  if (!ret.has_value()) {
    LOG(ERROR) << __func__
               << "() : JSONReader failed : " << ret.error().message;
    return false;
  }

  if (!ret->is_dict()) {
    LOG(ERROR) << __func__ << "() : Unexpected response type " << ret->type();
    return false;
  }

  for (auto arg: args) {
    std::string* value_ptr = ret->GetDict().FindString(arg.name);
    if (!value_ptr) {
      LOG(WARNING) << __func__ << "() : Absent argument '" << arg.name << "'";
      return false;
    } else {
      *arg.value = *value_ptr;
    }
  }
  return true;
}

void EmulatorDataSource::OnURLLoadComplete(
    const network::SimpleURLLoader* source,
    std::unique_ptr<std::string> response_body) {
  std::lock_guard<std::mutex> lock(mutex_);

  auto find_url_loader =
      [source](
          const std::unique_ptr<const network::SimpleURLLoader>& url_loader) {
        return url_loader.get() == source;
      };
  auto expectation_result =
      std::find_if(expectation_url_loaders_.begin(),
                   expectation_url_loaders_.end(), find_url_loader);
  auto url_result =
      std::find_if(url_loaders_.begin(), url_loaders_.end(), find_url_loader);

  int net_error = source->NetError();
  // 0xx HTTP errors don't exist so it's like an Unknown error code.
  int http_response_code = 1;
  if (auto* ri = source->ResponseInfo()) {
    http_response_code = ri->headers->response_code();
  }

  if (expectation_result != expectation_url_loaders_.end()) {
    if (net_error) {
      LOG(ERROR) << __func__ << "(): Request failed with network error code: "
                 << net::ErrorToString(net_error);
    }
    // Check for HTTP 5xx (server) errors
    if (http_response_code / 100 == 5) {
      LOG(ERROR) << __func__ << "(): Request failed with HTTP error code: "
                 << net::GetHttpReasonPhrase(
                        static_cast<net::HttpStatusCode>(http_response_code));
    }
    expectation_url_loaders_.erase(expectation_result);
    return;
  }

  if (url_result != url_loaders_.end()) {
    if (net_error) {
      LOG(ERROR) << __func__ << "(): Request failed with network error code: "
                 << net::ErrorToString(net_error);
    }
    // Check for HTTP 5xx (server) errors
    if (http_response_code / 100 == 5) {
      LOG(ERROR) << __func__ << "(): Request failed with HTTP error code: "
                 << net::GetHttpReasonPhrase(
                        static_cast<net::HttpStatusCode>(http_response_code));
    }
    if (http_response_code == net::HTTP_OK) {
      std::string loader_url = source->GetFinalURL().spec();
      URLData& url_data = urls_[loader_url];
      url_data.cached_data = *response_body;
      url_data.task_runner->PostTask(
          FROM_HERE, base::BindOnce(&EmulatorDataDelegate::DataUpdated,
                                    base::Unretained(url_data.delegate),
                                    url_data.url, *response_body));
    }
    url_loaders_.erase(url_result);
    return;
  }

  LOG(ERROR) << __func__ << "(): Wrong URLLoader";
  NOTREACHED();
}

base::WeakPtr<EmulatorDataSource> EmulatorDataSource::GetWeakPtr() {
  return weak_ptr_factory_.GetWeakPtr();
}

}  // namespace emulator
