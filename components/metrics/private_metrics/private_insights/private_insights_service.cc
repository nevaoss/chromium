// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/metrics/private_metrics/private_insights/private_insights_service.h"

#include <string>

#include "base/check.h"
#include "base/files/file_path.h"
#include "base/functional/bind.h"
#include "base/metrics/histogram_functions.h"
#include "base/sequence_checker.h"
#include "base/task/task_traits.h"
#include "base/task/thread_pool.h"
#include "base/time/time.h"
#include "components/metrics/metrics_pref_names.h"
#include "components/metrics/private_metrics/private_insights/fcp_event_publisher.h"
#include "components/metrics/private_metrics/private_insights/fcp_files.h"
#include "components/metrics/private_metrics/private_insights/fcp_flags.h"
#include "components/metrics/private_metrics/private_insights/fcp_log_manager.h"
#include "components/metrics/private_metrics/private_insights/fcp_simple_task_environment.h"
#include "components/metrics/private_metrics/private_insights/private_insights_features.h"
#include "components/prefs/pref_service.h"
#include "google_apis/google_api_keys.h"
#include "third_party/abseil-cpp/absl/status/statusor.h"
#include "third_party/federated_compute/src/fcp/client/fl_runner.h"

namespace private_insights {

namespace {

inline constexpr char kContextualCuesPopulationName[] =
    "private_insights/contextual_cues";

}  // namespace

PrivateInsightsService::PrivateInsightsService(
    PrefService* local_state,
    const base::FilePath& profile_dir)
    : local_state_(local_state), profile_dir_(profile_dir) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  CHECK(local_state_);
}

PrivateInsightsService::~PrivateInsightsService() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
}

void PrivateInsightsService::Init() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  pref_registrar_.Init(local_state_);
  pref_registrar_.Add(
      metrics::prefs::kMetricsReportingEnabled,
      base::BindRepeating(&PrivateInsightsService::OnMetricsChoiceChanged,
                          weak_ptr_factory_.GetWeakPtr()));
  OnMetricsChoiceChanged();
}

void PrivateInsightsService::OnMetricsChoiceChanged() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  if (PrivateInsightsMetricsServiceAccessor::IsMetricsReportingEnabled(
          local_state_)) {
    Start();
  } else {
    Stop();
  }
}

void PrivateInsightsService::Start() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  if (upload_timer_.IsRunning()) {
    return;
  }
  base::TimeDelta interval = kPrivateInsightsUploadInterval.Get();
  if (interval.is_positive()) {
    upload_timer_.Start(
        FROM_HERE, interval,
        base::BindRepeating(&PrivateInsightsService::TriggerUpload,
                            weak_ptr_factory_.GetWeakPtr()));
  }
}

void PrivateInsightsService::Stop() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  upload_timer_.Stop();
}

void PrivateInsightsService::Shutdown() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  Stop();
}

void PrivateInsightsService::TriggerUpload() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  if (is_upload_running_) {
    base::UmaHistogramEnumeration(kTriggerUploadOutcomeHistogram,
                                  TriggerUploadOutcome::kSkippedAlreadyRunning);
    return;
  }
  is_upload_running_ = true;

  base::ThreadPool::PostTaskAndReplyWithResult(
      FROM_HERE,
      {base::MayBlock(), base::TaskPriority::BEST_EFFORT,
       base::TaskShutdownBehavior::SKIP_ON_SHUTDOWN},
      base::BindOnce(&PrivateInsightsService::UploadBlocking, profile_dir_,
                     base::TimeTicks::Now()),
      base::BindOnce(&PrivateInsightsService::OnUploadComplete,
                     weak_ptr_factory_.GetWeakPtr()));

  base::UmaHistogramEnumeration(kTriggerUploadOutcomeHistogram,
                                TriggerUploadOutcome::kTaskPosted);
}

// static
bool PrivateInsightsService::UploadBlocking(const base::FilePath& profile_dir,
                                            base::TimeTicks trigger_time) {
  base::UmaHistogramTimes(kUploadPendingTimeHistogram,
                          base::TimeTicks::Now() - trigger_time);
  const std::string server_uri = kFcpServerUri.Get();
  if (server_uri.empty()) {
    return false;
  }
  base::TimeTicks upload_start_time = base::TimeTicks::Now();

  base::FilePath private_insights_dir =
      profile_dir.AppendASCII("PrivateInsights");
  base::FilePath base_dir = private_insights_dir.AppendASCII("base_dir");
  base::FilePath cache_dir = private_insights_dir.AppendASCII("cache_dir");

  FcpSimpleTaskEnvironment fcp_task_env(base_dir.AsUTF8Unsafe(),
                                        cache_dir.AsUTF8Unsafe(), {});
  FcpEventPublisher fcp_event_publisher;
  FcpFiles fcp_files;
  FcpLogManager fcp_log_manager;
  FcpFlags fcp_flags;

  FederatedComputationParams params{
      .task_env = &fcp_task_env,
      .event_publisher = &fcp_event_publisher,
      .files = &fcp_files,
      .log_manager = &fcp_log_manager,
      .flags = &fcp_flags,
      .federated_service_uri = server_uri,
      .api_key = google_apis::GetAPIKey(),
      .population_name = kContextualCuesPopulationName,
  };

  bool result = false;
  if (run_federated_computation_func) {
    result = run_federated_computation_func(params);
  }

  base::UmaHistogramTimes(kUploadTimeHistogram,
                          base::TimeTicks::Now() - upload_start_time);
  return result;
}

// static
bool PrivateInsightsService::RunFederatedComputation(
    const FederatedComputationParams& params) {
  absl::StatusOr<fcp::client::FLRunnerResult> statusor =  // nocheck
      fcp::client::RunFederatedComputation(
          /*env_deps=*/params.task_env,
          /*event_publisher=*/params.event_publisher,
          /*files=*/params.files,
          /*log_manager=*/params.log_manager,
          /*flags=*/params.flags,
          /*federated_service_uri=*/params.federated_service_uri,
          /*api_key=*/params.api_key,
          /*test_cert_path=*/"",
          // We don't use session_name per se, so just use population name
          // instead.
          /*session_name=*/params.population_name,
          /*population_name=*/params.population_name,
          /*retry_token=*/"",
          /*client_version=*/"",  // TODO(b/518646350): Add client version.
          /*client_attestation_measurement=*/"");
  return statusor.ok();
}

// static
PrivateInsightsService::RunFederatedComputationFunc
    PrivateInsightsService::run_federated_computation_func =
        &PrivateInsightsService::RunFederatedComputation;

void PrivateInsightsService::OnUploadComplete(bool _result) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  is_upload_running_ = false;
  // TODO(b/518646350): Handle the result of the upload.
}

// static
void PrivateInsightsMetricsServiceAccessor::
    SetForceIsMetricsReportingEnabledPrefLookupForTesting(bool value) {
  SetForceIsMetricsReportingEnabledPrefLookup(value);
}

}  // namespace private_insights
