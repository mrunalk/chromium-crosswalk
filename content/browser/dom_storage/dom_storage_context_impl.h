// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_DOM_STORAGE_DOM_STORAGE_CONTEXT_IMPL_H_
#define CONTENT_BROWSER_DOM_STORAGE_DOM_STORAGE_CONTEXT_IMPL_H_

#include <stdint.h>

#include <map>
#include <set>
#include <string>
#include <vector>

#include "base/atomic_sequence_num.h"
#include "base/files/file_path.h"
#include "base/gtest_prod_util.h"
#include "base/memory/ref_counted.h"
#include "base/observer_list.h"
#include "base/time/time.h"
#include "base/trace_event/memory_dump_provider.h"
#include "content/common/content_export.h"
#include "url/gurl.h"

namespace base {
class FilePath;
class NullableString16;
class Time;
}

namespace storage {
class SpecialStoragePolicy;
}

namespace content {

class DOMStorageArea;
class DOMStorageNamespace;
class DOMStorageSession;
class DOMStorageTaskRunner;
class SessionStorageDatabase;
struct LocalStorageUsageInfo;
struct SessionStorageUsageInfo;

// The Context is the root of an object containment hierarchy for
// Namespaces and Areas related to the owning profile.
// One instance is allocated in the main process for each profile,
// instance methods should be called serially in the background as
// determined by the task_runner. Specifically not on chrome's non-blocking
// IO thread since these methods can result in blocking file io.
//
// In general terms, the DOMStorage object relationships are...
//   Contexts (per-profile) own Namespaces which own Areas which share Maps.
//   Hosts(per-renderer) refer to Namespaces and Areas open in its renderer.
//   Sessions (per-tab) cause the creation and deletion of session Namespaces.
//
// Session Namespaces are cloned by initially making a shallow copy of
// all contained Areas, the shallow copies refer to the same refcounted Map,
// and does a deep copy-on-write if needed.
//
// Classes intended to be used by an embedder are DOMStorageContextImpl,
// DOMStorageHost, and DOMStorageSession. The other classes are for
// internal consumption.
class CONTENT_EXPORT DOMStorageContextImpl
    : public base::RefCountedThreadSafe<DOMStorageContextImpl>,
      public base::trace_event::MemoryDumpProvider {
 public:
  // An interface for observing Local and Session Storage events on the
  // background thread.
  class EventObserver {
   public:
    // |old_value| may be null on initial insert.
    virtual void OnDOMStorageItemSet(
        const DOMStorageArea* area,
        const base::string16& key,
        const base::string16& new_value,
        const base::NullableString16& old_value,
        const GURL& page_url) = 0;
    virtual void OnDOMStorageItemRemoved(
        const DOMStorageArea* area,
        const base::string16& key,
        const base::string16& old_value,
        const GURL& page_url) = 0;
    virtual void OnDOMStorageAreaCleared(
        const DOMStorageArea* area,
        const GURL& page_url) = 0;

   protected:
    virtual ~EventObserver() {}
  };

  // |localstorage_directory| and |sessionstorage_directory| may be empty
  // for incognito browser contexts.
  DOMStorageContextImpl(const base::FilePath& localstorage_directory,
                        const base::FilePath& sessionstorage_directory,
                        storage::SpecialStoragePolicy* special_storage_policy,
                        DOMStorageTaskRunner* task_runner);

  // Returns the directory path for localStorage, or an empty directory, if
  // there is no backing on disk.
  const base::FilePath& localstorage_directory() {
    return localstorage_directory_;
  }

  // Returns the directory path for sessionStorage, or an empty directory, if
  // there is no backing on disk.
  const base::FilePath& sessionstorage_directory() {
    return sessionstorage_directory_;
  }

  DOMStorageTaskRunner* task_runner() const { return task_runner_.get(); }
  DOMStorageNamespace* GetStorageNamespace(int64_t namespace_id);

  void GetLocalStorageUsage(std::vector<LocalStorageUsageInfo>* infos,
                            bool include_file_info);
  void GetSessionStorageUsage(std::vector<SessionStorageUsageInfo>* infos);
  void DeleteLocalStorage(const GURL& origin);
  void DeleteSessionStorage(const SessionStorageUsageInfo& usage_info);

  // Used by content settings to alter the behavior around
  // what data to keep and what data to discard at shutdown.
  // The policy is not so straight forward to describe, see
  // the implementation for details.
  void SetForceKeepSessionState() {
    force_keep_session_state_ = true;
  }

  // Called when the owning BrowserContext is ending.
  // Schedules the commit of any unsaved changes and will delete
  // and keep data on disk per the content settings and special storage
  // policies. Contained areas and namespaces will stop functioning after
  // this method has been called.
  void Shutdown();

  // Initiate the process of flushing (writing - not sync'ing) any unwritten
  // data managed by this instance. Flushing will start "soon".
  void Flush();

  // Methods to add, remove, and notify EventObservers.
  void AddEventObserver(EventObserver* observer);
  void RemoveEventObserver(EventObserver* observer);
  void NotifyItemSet(
      const DOMStorageArea* area,
      const base::string16& key,
      const base::string16& new_value,
      const base::NullableString16& old_value,
      const GURL& page_url);
  void NotifyItemRemoved(
      const DOMStorageArea* area,
      const base::string16& key,
      const base::string16& old_value,
      const GURL& page_url);
  void NotifyAreaCleared(
      const DOMStorageArea* area,
      const GURL& page_url);

  // May be called on any thread.
  int64_t AllocateSessionId();
  std::string AllocatePersistentSessionId();

  // Must be called on the background thread.
  void CreateSessionNamespace(int64_t namespace_id,
                              const std::string& persistent_namespace_id);
  void DeleteSessionNamespace(int64_t namespace_id, bool should_persist_data);
  void CloneSessionNamespace(int64_t existing_id,
                             int64_t new_id,
                             const std::string& new_persistent_id);

  // Starts backing sessionStorage on disk. This function must be called right
  // after DOMStorageContextImpl is created, before it's used.
  void SetSaveSessionStorageOnDisk();

  // Deletes all namespaces which don't have an associated DOMStorageNamespace
  // alive. This function is used for deleting possible leftover data after an
  // unclean exit.
  void StartScavengingUnusedSessionStorage();

  // base::trace_event::MemoryDumpProvider implementation.
  bool OnMemoryDump(const base::trace_event::MemoryDumpArgs& args,
                    base::trace_event::ProcessMemoryDump* pmd) override;

 private:
  friend class DOMStorageContextImplTest;
  FRIEND_TEST_ALL_PREFIXES(DOMStorageContextImplTest, Basics);
  friend class base::RefCountedThreadSafe<DOMStorageContextImpl>;
  typedef std::map<int64_t, scoped_refptr<DOMStorageNamespace>>
      StorageNamespaceMap;

  ~DOMStorageContextImpl() override;

  void ClearSessionOnlyOrigins();

  // For scavenging unused sessionStorages.
  void FindUnusedNamespaces();
  void FindUnusedNamespacesInCommitSequence(
      const std::set<std::string>& namespace_ids_in_use,
      const std::set<std::string>& protected_persistent_session_ids);
  void DeleteNextUnusedNamespace();
  void DeleteNextUnusedNamespaceInCommitSequence();

  // Collection of namespaces keyed by id.
  StorageNamespaceMap namespaces_;

  // Where localstorage data is stored, maybe empty for the incognito use case.
  base::FilePath localstorage_directory_;

  // Where sessionstorage data is stored, maybe empty for the incognito use
  // case. Always empty until the file-backed session storage feature is
  // implemented.
  base::FilePath sessionstorage_directory_;

  // Used to schedule sequenced background tasks.
  scoped_refptr<DOMStorageTaskRunner> task_runner_;

  // List of objects observing local storage events.
  base::ObserverList<EventObserver> event_observers_;

  // We use a 32 bit identifier for per tab storage sessions.
  // At a tab per second, this range is large enough for 68 years.
  // The offset is to more quickly detect the error condition where
  // an id related to one context is mistakenly used in another.
  base::AtomicSequenceNumber session_id_sequence_;
  const int session_id_offset_;

  bool is_shutdown_;
  bool force_keep_session_state_;
  scoped_refptr<storage::SpecialStoragePolicy> special_storage_policy_;
  scoped_refptr<SessionStorageDatabase> session_storage_database_;

  // For cleaning up unused namespaces gradually.
  bool scavenging_started_;
  std::vector<std::string> deletable_persistent_namespace_ids_;

  // Persistent namespace IDs to protect from gradual deletion (they will
  // be needed for session restore).
  std::set<std::string> protected_persistent_session_ids_;

  // Mapping between persistent namespace IDs and namespace IDs for
  // sessionStorage.
  std::map<std::string, int64_t> persistent_namespace_id_to_namespace_id_;
};

}  // namespace content

#endif  // CONTENT_BROWSER_DOM_STORAGE_DOM_STORAGE_CONTEXT_IMPL_H_
