// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_DOWNLOAD_MODEL_DOWNLOAD_RECORD_STORE_H_
#define IOS_CHROME_BROWSER_DOWNLOAD_MODEL_DOWNLOAD_RECORD_STORE_H_

#import <map>
#import <memory>
#import <optional>
#import <string>
#import <string_view>
#import <vector>

#import "base/files/file_path.h"
#import "base/sequence_checker.h"
#import "ios/chrome/browser/download/model/download_filter_util.h"
#import "ios/chrome/browser/download/model/download_record.h"
#import "ios/chrome/browser/download/model/download_record_query.h"
#import "ios/web/public/download/download_task.h"

class DownloadRecordDatabase;

// Sequence-bound owner of the SQLite-backed download record database and the
// in-memory record cache. All public methods MUST be called on the database
// sequence (the SequencedTaskRunner that the owning service binds at
// construction). Thread safety is achieved by sequence affinity, not locks.
//
// Lifetime: DownloadRecordServiceImpl owns a
// `base::SequenceBound<DownloadRecordStore>`. SequenceBound posts both the
// store's construction and destruction onto the bound sequence, so any
// in-flight DB-sequence task observes a still-live DownloadRecordStore even
// if the owning service has already been torn down on the main thread.
//
// See crbug.com/524030520 for the original use-after-free risk this design
// addresses.
class DownloadRecordStore {
 public:
  DownloadRecordStore();

  DownloadRecordStore(const DownloadRecordStore&) = delete;
  DownloadRecordStore& operator=(const DownloadRecordStore&) = delete;

  ~DownloadRecordStore();

  // Initializes the SQLite database under
  // `<profile_path>/download_record_db/DownloadRecord`. No-op (and the store
  // remains in an "uninitialized" state) if the directory cannot be created
  // or if database initialization fails.
  void InitializeDatabase(const base::FilePath& profile_path);

  // ---- Startup paths -----------------------------------------------------

  // Legacy startup loader (flag-OFF path). Loads every persisted row into
  // the in-memory cache and flips any kInProgress/kNotStarted row to
  // kFailed. Only used when `IsDownloadListPaginationEnabled()` is false.
  // TODO(crbug.com/524790428): Remove this legacy startup path (and the
  // cache-based readers) once the pagination flag has shipped to stable
  // and is retired.
  void LoadHistoricalRecords();

  // Pagination-aware startup cleanup (flag-ON path). Issues a single
  // UPDATE ... WHERE state IN (kInProgress, kNotStarted) against the DB
  // (no full-table load).
  void MarkUnfinishedDownloadsAsFailed();

  // ---- Database CRUD operations ------------------------------------------

  // Inserts `record`. For incognito records (`is_incognito == true`) the
  // record is held only in the in-memory cache and never persisted.
  // Returns true on success.
  bool InsertRecord(const DownloadRecord& record);

  // Updates an existing record identified by `updated_record.download_id`.
  // Preserves `created_time` from the cached record. Returns the merged
  // record on success — whether or not the DB was actually written
  // (incognito / progress-only updates skip the DB but still refresh the
  // cache). Returns std::nullopt if no cached record exists or a required
  // DB write failed.
  std::optional<DownloadRecord> UpdateRecord(
      const DownloadRecord& updated_record);

  // Bulk-update legacy helper (flag-OFF only). Updates `download_ids` to
  // `new_state` in a single DB transaction and refreshes cache entries.
  // TODO(crbug.com/524790428): Remove once the pagination flag has shipped
  // to stable and is retired.
  bool UpdateRecordsState(const std::vector<std::string>& download_ids,
                          web::DownloadTask::State new_state);

  // Looks up `download_id` in the cache, applies `file_path` to a copy of
  // it, and routes through `UpdateRecord`. Returns the merged record on
  // success, std::nullopt otherwise.
  std::optional<DownloadRecord> UpdateFilePathInRecord(
      const std::string& download_id,
      const base::FilePath& file_path);

  // Deletes the record identified by `id`. For non-incognito records,
  // attempts a DB delete first; on DB success the cache entry is also
  // erased. Returns true on success (or if the record never existed).
  bool DeleteRecord(std::string_view id);

  // ---- Cache reads -------------------------------------------------------

  // Returns a snapshot of all records currently in the in-memory cache.
  std::vector<DownloadRecord> GetAllFromCache();

  // Returns the cached record for `download_id`, or std::nullopt if not in
  // the cache.
  std::optional<DownloadRecord> GetByIdFromCache(std::string_view download_id);

  // ---- Database paginated reads ------------------------------------------

  // Cold-DB readers used by the paginated UI. Both bypass the in-memory
  // cache by design — they exist so the flag-ON path can serve very large
  // download histories without paying the O(N) load cost of
  // LoadHistoricalRecords. Returns an empty vector / 0 if the DB is not
  // initialized.
  std::vector<DownloadRecord> GetDownloadsPage(
      const DownloadRecordQuery& query) const;
  size_t GetDownloadsCount(const DownloadRecordQuery& query) const;

 private:
  // DRY helper for the repeated `database_ && database_->IsInitialized()`
  // check used by every mutator below.
  bool IsDatabaseReady() const;

  // In-memory cache of download records. On the legacy flag-OFF path it
  // mirrors every persisted row plus any incognito-only records. On the
  // flag-ON path this map stays empty by design — paginated readers go
  // cold-DB instead. Accessed only on `sequence_checker_`.
  // `std::less<>` enables heterogeneous lookup (e.g. `find(std::string_view)`)
  // without constructing a temporary `std::string`.
  // TODO(crbug.com/524790428): Remove this map and its legacy population
  // path once the pagination flag has shipped to stable and is retired.
  std::map<std::string, DownloadRecord, std::less<>> record_cache_;

  // SQLite-backed database. Owned and accessed only on
  // `sequence_checker_`. Held by unique_ptr so that an Init() failure
  // leaves this null and callers can short-circuit via
  // `IsDatabaseReady()`.
  std::unique_ptr<DownloadRecordDatabase> database_;

  // Sequence checker that pins all public and private methods to a
  // single sequence. The store is constructed on the database sequence by
  // `base::SequenceBound`; the checker is detached at construction and
  // rebinds on the first method call (which always happens on the bound
  // sequence) so all subsequent calls happen on that same sequence.
  SEQUENCE_CHECKER(sequence_checker_);
};

#endif  // IOS_CHROME_BROWSER_DOWNLOAD_MODEL_DOWNLOAD_RECORD_STORE_H_
