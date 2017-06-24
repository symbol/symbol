#pragma once
#include "HashCacheDelta.h"
#include "HashCacheView.h"
#include "catapult/cache/SynchronizedCache.h"

namespace catapult { namespace cache {

	/// Cache composed of timestamped hashes of (transaction) entities.
	/// The cache can be pruned according to the retention time.
	class BasicHashCache : public utils::MoveOnly {
	public:
		using CacheViewType = HashCacheView;
		using CacheDeltaType = HashCacheDelta;
		using CacheReadOnlyType = hash_cache_types::CacheReadOnlyType;

	public:
		/// Creates a hash cache with the specified retention time (\a retentionTime).
		explicit BasicHashCache(const utils::TimeSpan& retentionTime) : m_retentionTime(retentionTime)
		{}

	public:
		/// Returns a locked view based on this cache.
		CacheViewType createView() const {
			return CacheViewType(m_retentionTime, m_orderedSet);
		}

		/// Returns a locked cache delta based on this cache.
		CacheDeltaType createDelta() {
			return createDelta(m_orderedSet.rebase());
		}

		/// Returns a lockable cache delta based on this cache but without the ability
		/// to commit any changes to the original cache.
		CacheDeltaType createDetachedDelta() const {
			return createDelta(m_orderedSet.rebaseDetached());
		}

		/// Commits all pending changes to the underlying storage.
		void commit(const CacheDeltaType& delta) {
			m_orderedSet.commit(delta.pruningBoundary());
		}

	private:
		inline CacheDeltaType createDelta(const hash_cache_types::BaseSetDeltaPointerType& pDelta) const {
			return CacheDeltaType(m_retentionTime, pDelta);
		}

	private:
		utils::TimeSpan m_retentionTime;
		hash_cache_types::BaseSetType m_orderedSet;
	};

	/// Synchronized cache composed of timestamped hashes of (transaction) entities.
	class HashCache : public SynchronizedCache<BasicHashCache> {
	public:
		/// The unique cache identifier.
		static constexpr size_t Id = 2;

		/// The cache friendly name.
		static constexpr auto Name = "HashCache";

	public:
		/// Creates a hash cache with the specified retention time (\a retentionTime).
		explicit HashCache(const utils::TimeSpan& retentionTime) : SynchronizedCache<BasicHashCache>(BasicHashCache(retentionTime))
		{}
	};
}}
