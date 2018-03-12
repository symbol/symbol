#pragma once
#include "HashCacheTypes.h"
#include "catapult/cache/CacheMixins.h"
#include "catapult/cache/ReadOnlySimpleCache.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"

namespace catapult { namespace cache {

	/// Basic delta on top of the hash cache.
	class BasicHashCacheDelta
			: public utils::MoveOnly
			, public SizeMixin<HashCacheTypes::BaseSetDeltaType>
			, public ContainsMixin<HashCacheTypes::BaseSetDeltaType, HashCacheDescriptor>
			, public BasicInsertRemoveMixin<HashCacheTypes::BaseSetDeltaType, HashCacheDescriptor> {
	public:
		using ReadOnlyView = HashCacheTypes::CacheReadOnlyType;
		using ValueType = HashCacheDescriptor::ValueType;

	public:
		/// Creates a delta based on the hash set (\a pHashes) and \a options.
		BasicHashCacheDelta(const HashCacheTypes::BaseSetDeltaPointerType& pHashes, const HashCacheTypes::Options& options);

	public:
		/// Gets the retention time for the cache.
		utils::TimeSpan retentionTime() const;

		/// Gets the pruning boundary that is used during commit.
		deltaset::PruningBoundary<ValueType> pruningBoundary() const;

	public:
		/// Removes all timestamped hashes that have timestamps prior to the given \a timestamp minus the retention time.
		void prune(Timestamp timestamp);

	private:
		HashCacheTypes::BaseSetDeltaPointerType m_pOrderedDelta;
		utils::TimeSpan m_retentionTime;
		deltaset::PruningBoundary<ValueType> m_pruningBoundary;
	};

	/// Delta on top of the hash cache.
	class HashCacheDelta : public ReadOnlyViewSupplier<BasicHashCacheDelta> {
	public:
		/// Creates a delta based on the hash set (\a pHashes) and \a options.
		HashCacheDelta(const HashCacheTypes::BaseSetDeltaPointerType& pHashes, const HashCacheTypes::Options& options)
				: ReadOnlyViewSupplier(pHashes, options)
		{}
	};
}}
