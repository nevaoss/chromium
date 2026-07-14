// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/browser/download/model/download_record_store.h"

#import <tuple>
#import <utility>

#import "base/files/file_util.h"
#import "ios/chrome/browser/download/model/download_record_database.h"

namespace {

// Determines whether a download record update should be persisted to the
// database by comparing critical fields between the new and cached
// records. Incognito records are never persisted; non-incognito records
// only need a DB write when a non-progress field actually changed.
bool ShouldPersistUpdate(const DownloadRecord& new_record,
                         const DownloadRecord& cached_record) {
  // Incognito records are not persistently stored.
  if (cached_record.is_incognito) {
    return false;
  }

  // Persist only if critical fields have changed.
  // Progress fields (received_bytes, progress_percent) are not persisted to
  // database.
  return !new_record.EqualsExcludingProgress(cached_record);
}

}  // namespace

DownloadRecordStore::DownloadRecordStore() {
  // `base::SequenceBound` constructs the store on the database sequence,
  // but the SEQUENCE_CHECKER is unconditionally detached at construction
  // so it rebinds on the first method call rather than on whichever
  // sequence happened to construct the object.
  DETACH_FROM_SEQUENCE(sequence_checker_);
}

DownloadRecordStore::~DownloadRecordStore() {
  // `base::SequenceBound` posts the destructor onto the bound database
  // sequence, so it always runs on the same sequence as every other
  // method.
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
}

void DownloadRecordStore::InitializeDatabase(
    const base::FilePath& profile_path) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  base::FilePath downloads_dir =
      profile_path.Append(FILE_PATH_LITERAL("download_record_db"));
  base::FilePath db_path =
      downloads_dir.Append(FILE_PATH_LITERAL("DownloadRecord"));

  if (!base::CreateDirectory(downloads_dir)) {
    return;
  }

  auto database = std::make_unique<DownloadRecordDatabase>(db_path);
  sql::InitStatus init_status = database->Init();

  if (init_status == sql::INIT_OK) {
    database_ = std::move(database);
  }
}

void DownloadRecordStore::LoadHistoricalRecords() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  if (!IsDatabaseReady()) {
    return;
  }

  record_cache_.clear();

  std::vector<DownloadRecord> all_records = database_->GetAllDownloadRecords();
  for (const auto& record : all_records) {
    record_cache_[record.download_id] = record;
  }

  // Mark any record stuck in kInProgress / kNotStarted as kFailed. This
  // mirrors the original CleanupInconsistentStates legacy helper.
  std::vector<std::string> records_to_fix;
  for (const auto& [id, record] : record_cache_) {
    if (record.state == web::DownloadTask::State::kInProgress ||
        record.state == web::DownloadTask::State::kNotStarted) {
      records_to_fix.push_back(id);
    }
  }

  if (records_to_fix.empty()) {
    return;
  }

  UpdateRecordsState(records_to_fix, web::DownloadTask::State::kFailed);
}

void DownloadRecordStore::MarkUnfinishedDownloadsAsFailed() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  if (!IsDatabaseReady()) {
    return;
  }

  // Single SQL UPDATE — no full-table load. Any record found in
  // kInProgress / kNotStarted at startup is treated as interrupted by
  // app termination and flipped to kFailed.
  //
  // Return value is intentionally discarded: a transient SQL failure
  // here is self-healing (the next OnDownload* update for the row, or
  // the next session's identical UPDATE, will repair the state) and no
  // caller depends on its success.
  std::ignore = database_->MarkUnfinishedDownloadsAsFailed();

  // `record_cache_` is intentionally not populated here. It is only
  // populated by `LoadHistoricalRecords` on the flag-OFF path; on the
  // flag-ON path it stays empty by design. This keeps service
  // construction O(1) instead of O(N) on the persisted-row count.
}

bool DownloadRecordStore::InsertRecord(const DownloadRecord& record) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  bool should_persist = !record.is_incognito;

  if (!should_persist) {
    record_cache_[record.download_id] = record;
    return true;
  }

  if (!IsDatabaseReady()) {
    return false;
  }

  if (database_->InsertDownloadRecord(record)) {
    record_cache_[record.download_id] = record;
    return true;
  }

  return false;
}

std::optional<DownloadRecord> DownloadRecordStore::UpdateRecord(
    const DownloadRecord& updated_record) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  auto existing_record_opt = GetByIdFromCache(updated_record.download_id);
  if (!existing_record_opt.has_value()) {
    return std::nullopt;
  }

  // Preserve created_time from existing record.
  DownloadRecord record_to_update = updated_record;
  record_to_update.created_time = existing_record_opt.value().created_time;

  // Determine if we need to persist this update to database.
  bool should_persist =
      ShouldPersistUpdate(record_to_update, existing_record_opt.value());

  if (!should_persist) {
    record_cache_[record_to_update.download_id] = record_to_update;
    return record_to_update;
  }

  if (!IsDatabaseReady()) {
    return std::nullopt;
  }

  if (database_->UpdateDownloadRecord(record_to_update)) {
    record_cache_[record_to_update.download_id] = record_to_update;
    return record_to_update;
  }

  return std::nullopt;
}

bool DownloadRecordStore::UpdateRecordsState(
    const std::vector<std::string>& download_ids,
    web::DownloadTask::State new_state) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  if (download_ids.empty()) {
    // Empty list is considered successful.
    return true;
  }

  if (!IsDatabaseReady()) {
    return false;
  }

  if (database_->UpdateDownloadRecordsState(download_ids, new_state)) {
    // Updates cache for all successfully updated records.
    for (const std::string& download_id : download_ids) {
      auto it = record_cache_.find(download_id);
      if (it != record_cache_.end()) {
        it->second.state = new_state;
      }
    }
    return true;
  }

  return false;
}

std::optional<DownloadRecord> DownloadRecordStore::UpdateFilePathInRecord(
    const std::string& download_id,
    const base::FilePath& file_path) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  auto existing_record_opt = GetByIdFromCache(download_id);
  if (!existing_record_opt.has_value()) {
    return std::nullopt;
  }

  // Create updated record with new file path.
  DownloadRecord updated_record = existing_record_opt.value();
  updated_record.file_path = file_path;

  return UpdateRecord(updated_record);
}

bool DownloadRecordStore::DeleteRecord(std::string_view id) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  // Check if record exists in cache.
  auto it = record_cache_.find(std::string(id));
  if (it == record_cache_.end()) {
    // Consider this a success since the record doesn't exist anyway.
    return true;
  }

  const DownloadRecord& record = it->second;
  bool should_persist = !record.is_incognito;

  if (!should_persist) {
    record_cache_.erase(it);
    return true;
  }

  if (!IsDatabaseReady()) {
    return false;
  }

  if (database_->DeleteDownloadRecord(std::string(id))) {
    record_cache_.erase(it);
    return true;
  }

  return false;
}

std::vector<DownloadRecord> DownloadRecordStore::GetAllFromCache() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  std::vector<DownloadRecord> records;
  records.reserve(record_cache_.size());

  for (const auto& [id, record] : record_cache_) {
    records.push_back(record);
  }

  return records;
}

std::optional<DownloadRecord> DownloadRecordStore::GetByIdFromCache(
    std::string_view download_id) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  auto it = record_cache_.find(std::string(download_id));
  if (it != record_cache_.end()) {
    return it->second;
  }

  return std::nullopt;
}

std::vector<DownloadRecord> DownloadRecordStore::GetDownloadsPage(
    const DownloadRecordQuery& query) const {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  if (!IsDatabaseReady()) {
    return {};
  }
  return database_->GetDownloadRecordsPage(query);
}

size_t DownloadRecordStore::GetDownloadsCount(
    const DownloadRecordQuery& query) const {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  if (!IsDatabaseReady()) {
    return 0;
  }
  return static_cast<size_t>(database_->GetDownloadRecordsCount(query));
}

bool DownloadRecordStore::IsDatabaseReady() const {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  return database_ && database_->IsInitialized();
}
