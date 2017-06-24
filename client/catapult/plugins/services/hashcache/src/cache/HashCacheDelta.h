#pragma once
#include "HashCacheTypes.h"
#include "catapult/cache/ReadOnlySimpleCache.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"
#include "catapult/deltaset/BaseSetDelta.h"
#include "catapult/utils/SpinReaderWriterLock.h"
#include "catapult/utils/TimeSpan.h"

namespace catapult { namespace cache {

	/// Basic delta on top of the hash cache.
	class BasicHashCacheDelta : public utils::MoveOnly {
	public:
		using ReadOnlyView = hash_cache_types::CacheReadOnlyType;
		using ValueType = hash_cache_types::ValueType;

	public:
		/// Creates a delta with the specified retention time (\a retentionTime) based on the specified delta (\a pDelta).
		BasicHashCacheDelta(const utils::TimeSpan& retentionTime, const hash_cache_types::BaseSetDeltaPointerType& pDelta)
				: m_retentionTime(retentionTime)
				, m_pOrderedDelta(pDelta)
		{}

	public:
		/// Gets the retention time for the cache.
		utils::TimeSpan retentionTime() const {
			return m_retentionTime;
		}

		/// Gets the size of the set.
		size_t size() const {
			return m_pOrderedDelta->size();
		}

		/// Searches for the given timestamped hash (\a timestampedHash) in the cache.
		/// Returns \c true if it is found or \c false if it is not found.
		bool contains(const ValueType& timestampedHash) const {
			return m_pOrderedDelta->contains(timestampedHash);
		}

		/// Inserts a timestamped hash (\a timestampedHash).
		void insert(const ValueType& timestampedHash) {
			m_pOrderedDelta->insert(timestampedHash);
		}

		/// Inserts a timestamped hash into the set given a \a timestamp and a \a hash.
		void insert(Timestamp timestamp, const Hash256& hash) {
			m_pOrderedDelta->emplace(timestamp, hash);
		}

		/// Removes a timestamped \a hash from the set.
		void remove(const ValueType& timestampedHash) {
			m_pOrderedDelta->remove(timestampedHash);
		}

	public:
		/// Removes all timestamped hashes that have timestamps prior to the given \a timestamp minus the
		/// retention time.
		void prune(Timestamp timestamp);

		/// Gets the pruning boundary that is used during commit.
		deltaset::PruningBoundary<ValueType> pruningBoundary() const {
			return m_pruningBoundary;
		}

	private:
		utils::TimeSpan m_retentionTime;
		hash_cache_types::BaseSetDeltaPointerType m_pOrderedDelta;
		deltaset::PruningBoundary<ValueType> m_pruningBoundary;
	};

	/// Delta on top of the hash cache.
	class HashCacheDelta : public ReadOnlyViewSupplier<BasicHashCacheDelta> {
	public:
		/// Creates a delta with the specified retention time (\a retentionTime) based on the specified delta (\a pDelta).
		HashCacheDelta(const utils::TimeSpan& retentionTime, const hash_cache_types::BaseSetDeltaPointerType& pDelta)
				: ReadOnlyViewSupplier(retentionTime, pDelta)
		{}
	};
}}
