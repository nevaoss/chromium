// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_METRICS_PRIVATE_METRICS_PRIVATE_INSIGHTS_PRIVATE_INSIGHTS_SERVICE_H_
#define COMPONENTS_METRICS_PRIVATE_METRICS_PRIVATE_INSIGHTS_PRIVATE_INSIGHTS_SERVICE_H_

#include <string>

#include "base/component_export.h"
#include "base/files/file_path.h"
#include "base/gtest_prod_util.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/scoped_refptr.h"
#include "base/memory/weak_ptr.h"
#include "base/sequence_checker.h"
#include "base/time/time.h"
#include "base/timer/timer.h"
#include "components/keyed_service/core/keyed_service.h"
#include "components/metrics/metrics_service_accessor.h"
#include "components/prefs/pref_change_registrar.h"

class PrefService;

namespace private_insights {

class FcpEventPublisher;
class FcpFiles;
class FcpFlags;
class FcpLogManager;
class FcpSimpleTaskEnvironment;

class PrivateInsightsService;

class COMPONENT_EXPORT(PRIVATE_INSIGHTS) PrivateInsightsMetricsServiceAccessor
    : public metrics::MetricsServiceAccessor {
 public:
  static void SetForceIsMetricsReportingEnabledPrefLookupForTesting(bool value);

 private:
  friend class PrivateInsightsService;
};

inline constexpr char kTriggerUploadOutcomeHistogram[] =
    "PrivateMetrics.PrivateInsights.TriggerUploadOutcome";
inline constexpr char kUploadPendingTimeHistogram[] =
    "PrivateMetrics.PrivateInsights.Upload.PendingTime";
inline constexpr char kUploadTimeHistogram[] =
    "PrivateMetrics.PrivateInsights.Upload.Time";

class COMPONENT_EXPORT(PRIVATE_INSIGHTS) PrivateInsightsService
    : public KeyedService {
 public:
  struct FederatedComputationParams {
    raw_ptr<FcpSimpleTaskEnvironment> task_env;
    raw_ptr<FcpEventPublisher> event_publisher;
    raw_ptr<FcpFiles> files;
    raw_ptr<FcpLogManager> log_manager;
    raw_ptr<FcpFlags> flags;
    std::string federated_service_uri;
    std::string api_key;
    std::string session_name;
    std::string population_name;
  };

  using RunFederatedComputationFunc =
      bool (*)(const FederatedComputationParams& params);

  // LINT.IfChange(PrivateInsightsTriggerUploadOutcome)
  enum class TriggerUploadOutcome {
    kSkippedAlreadyRunning = 0,
    kTaskPosted = 1,
    kMaxValue = kTaskPosted,
  };
  // LINT.ThenChange(//tools/metrics/histograms/metadata/private_metrics/enums.xml:PrivateInsightsTriggerUploadOutcome)

  PrivateInsightsService(PrefService* local_state,
                         const base::FilePath& profile_dir);
  ~PrivateInsightsService() override;

  PrivateInsightsService(const PrivateInsightsService&) = delete;
  PrivateInsightsService& operator=(const PrivateInsightsService&) = delete;

  void Init();
  void Start();
  void Stop();

  // KeyedService:
  void Shutdown() override;

  static void SetRunFederatedComputationForTesting(
      RunFederatedComputationFunc func) {
    run_federated_computation_func =
        func ? func : &PrivateInsightsService::RunFederatedComputation;
  }

 private:
  void OnMetricsChoiceChanged();

  void TriggerUpload();

  // Runs on a background thread pool sequence (allows blocking).
  static bool UploadBlocking(const base::FilePath& profile_dir,
                             base::TimeTicks trigger_time);

  static bool RunFederatedComputation(const FederatedComputationParams& params);

  static RunFederatedComputationFunc run_federated_computation_func;

  void OnUploadComplete(bool result);

  raw_ptr<PrefService> local_state_ = nullptr;
  base::FilePath profile_dir_;
  PrefChangeRegistrar pref_registrar_;

  bool is_upload_running_ = false;
  base::RepeatingTimer upload_timer_;

  SEQUENCE_CHECKER(sequence_checker_);
  base::WeakPtrFactory<PrivateInsightsService> weak_ptr_factory_{this};

  FRIEND_TEST_ALL_PREFIXES(PrivateInsightsServiceTest,
                           TriggerUploadSkipsPostingTaskWhenAlreadyRunning);
  FRIEND_TEST_ALL_PREFIXES(PrivateInsightsServiceTest, MetricsChoiceCoupling);
  FRIEND_TEST_ALL_PREFIXES(PrivateInsightsServiceTest,
                           MetricsChoiceRespectedOnStartup);
  FRIEND_TEST_ALL_PREFIXES(PrivateInsightsServiceTest,
                           UploadSkippedWhenServerUriEmpty);
};

}  // namespace private_insights

#endif  // COMPONENTS_METRICS_PRIVATE_METRICS_PRIVATE_INSIGHTS_PRIVATE_INSIGHTS_SERVICE_H_
