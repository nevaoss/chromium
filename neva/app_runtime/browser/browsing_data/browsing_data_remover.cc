// Copyright 2017-2018 LG Electronics, Inc.
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

#include "browser/browsing_data/browsing_data_remover.h"

#include "base/functional/bind.h"
#include "components/web_cache/browser/web_cache_manager.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/storage_partition.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_request_context_getter.h"
#include "neva/app_runtime/browser/app_runtime_browser_context.h"
#include "services/network/network_context.h"
#include "services/network/public/mojom/clear_data_filter.mojom.h"
#include "third_party/blink/public/common/storage_key/storage_key.h"
#include "url/gurl.h"

namespace neva_app_runtime {

// static
BrowsingDataRemover::TimeRange BrowsingDataRemover::Unbounded() {
  return TimeRange(base::Time(), base::Time::Max());
}

BrowsingDataRemover* BrowsingDataRemover::GetForStoragePartition(
    content::StoragePartition* partition) {
  return new BrowsingDataRemover(partition);
}

BrowsingDataRemover::BrowsingDataRemover(
    content::StoragePartition* storage_partition)
    : storage_partition_(storage_partition),
      waiting_for_clear_channel_ids_(false),
      waiting_for_clear_cache_(false),
      waiting_for_clear_code_cache_(false),
      waiting_for_clear_storage_partition_data_(false),
      weak_ptr_factory_(this) {}

BrowsingDataRemover::~BrowsingDataRemover() {}

void BrowsingDataRemover::Remove(const TimeRange& time_range, int remove_mask) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  DCHECK_NE(base::Time(), time_range.end);

  // Start time to delete from.
  base::Time delete_begin = time_range.begin;

  // End time to delete to.
  base::Time delete_end = time_range.end;

  // TODO (jani) REMOVE_HISTORY support this?
  // Currently we don't track this in WebView

  // TODO (jani) REMOVE_DOWNLOADS currently we don't provide DownloadManager
  // for webview yet

  // TODO(neva): Supporting of Channel IDs was removed from upstream, see:
  // https://chromium.googlesource.com/chromium/src/+/695810675e53f811a2a7d976a826881fd6f2cd4f
  // This function should be revised according to last upstream changes

  uint32_t storage_partition_remove_mask = 0;
  if (remove_mask & REMOVE_COOKIES) {
    storage_partition_remove_mask |=
        content::StoragePartition::REMOVE_DATA_MASK_COOKIES;
  }
  if (remove_mask & REMOVE_LOCAL_STORAGE) {
    storage_partition_remove_mask |=
        content::StoragePartition::REMOVE_DATA_MASK_LOCAL_STORAGE;
  }
  if (remove_mask & REMOVE_INDEXEDDB) {
    storage_partition_remove_mask |=
        content::StoragePartition::REMOVE_DATA_MASK_INDEXEDDB;
  }
  if (remove_mask & REMOVE_SERVICE_WORKERS) {
    storage_partition_remove_mask |=
        content::StoragePartition::REMOVE_DATA_MASK_SERVICE_WORKERS;
  }
  if (remove_mask & REMOVE_CACHE_STORAGE) {
    storage_partition_remove_mask |=
        content::StoragePartition::REMOVE_DATA_MASK_CACHE_STORAGE;
  }
  if (remove_mask & REMOVE_FILE_SYSTEMS) {
    storage_partition_remove_mask |=
        content::StoragePartition::REMOVE_DATA_MASK_FILE_SYSTEMS;
  }

  if (remove_mask & REMOVE_PLUGIN_DATA) {
    // TODO (jani) Do we need this?
  }

  if (remove_mask & REMOVE_SESSION_COOKIES) {
    DeleteCookies(
        delete_begin,
        network::mojom::CookieDeletionSessionControl::SESSION_COOKIES);
  }

  if (remove_mask & REMOVE_PERSISTENT_COOKIES) {
    DeleteCookies(
        delete_begin,
        network::mojom::CookieDeletionSessionControl::PERSISTENT_COOKIES);
  }

  if (remove_mask & REMOVE_CACHE) {
    // Tell the renderers to clear their cache.
    web_cache::WebCacheManager::GetInstance()->ClearCache();

    waiting_for_clear_cache_ = true;
    network::mojom::NetworkContext* network_context =
        storage_partition_->GetNetworkContext();
    network_context->ClearHttpCache(
        delete_begin, delete_end, nullptr,
        base::BindOnce(&BrowsingDataRemover::OnClearedCache,
                       weak_ptr_factory_.GetWeakPtr()));

    // Tell the shader disk cache to clear.
    storage_partition_remove_mask |=
        content::StoragePartition::REMOVE_DATA_MASK_SHADER_CACHE;
  }

  // Content Decryption Modules used by Encrypted Media store licenses in a
  // private filesystem. These are different than content licenses used by
  // Flash (which are deleted father down in this method).
  if (remove_mask & REMOVE_MEDIA_LICENSES) {
    storage_partition_remove_mask |=
        content::StoragePartition::REMOVE_DATA_MASK_MEDIA_LICENSES;
  }

  if (remove_mask & REMOVE_CODE_CACHE) {
    waiting_for_clear_code_cache_ = true;
    storage_partition_->ClearCodeCaches(
        delete_begin, delete_end, base::RepeatingCallback<bool(const GURL&)>(),
        base::BindOnce(&BrowsingDataRemover::OnClearedCodeCache,
                       weak_ptr_factory_.GetWeakPtr()));
  }

  if (storage_partition_remove_mask) {
    waiting_for_clear_storage_partition_data_ = true;

    storage_partition_->ClearData(
        storage_partition_remove_mask, blink::StorageKey(), delete_begin,
        delete_end,
        base::BindOnce(&BrowsingDataRemover::OnClearedStoragePartitionData,
                       weak_ptr_factory_.GetWeakPtr()));
  }
}

void BrowsingDataRemover::DeleteCookies(
    base::Time& delete_begin,
    network::mojom::CookieDeletionSessionControl session_control) {
  network::mojom::CookieManager* cookie_manager =
      storage_partition_->GetCookieManagerForBrowserProcess();
  network::mojom::CookieDeletionFilterPtr filter =
      network::mojom::CookieDeletionFilter::New();
  filter->created_after_time = delete_begin;
  filter->session_control = session_control;
  cookie_manager->DeleteCookies(std::move(filter), base::DoNothing());
}

void BrowsingDataRemover::NotifyAndDelete() {
  base::SingleThreadTaskRunner::GetCurrentDefault()->DeleteSoon(FROM_HERE,
                                                                this);
}

void BrowsingDataRemover::NotifyAndDeleteIfDone() {
  if (!AllDone())
    return;

  NotifyAndDelete();
}

void BrowsingDataRemover::OnClearedCache() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  waiting_for_clear_cache_ = false;
  NotifyAndDeleteIfDone();
}

void BrowsingDataRemover::OnClearedStoragePartitionData() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  waiting_for_clear_storage_partition_data_ = false;
  NotifyAndDeleteIfDone();
}

void BrowsingDataRemover::OnClearedCodeCache() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  waiting_for_clear_code_cache_ = false;
  NotifyAndDeleteIfDone();
}

bool BrowsingDataRemover::AllDone() {
  return !waiting_for_clear_channel_ids_ && !waiting_for_clear_cache_ &&
         !waiting_for_clear_code_cache_ &&
         !waiting_for_clear_storage_partition_data_;
}

}  // namespace neva_app_runtime
