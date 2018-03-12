#pragma once
#include "catapult/cache/CacheDescriptorAdapters.h"
#include "catapult/state/TimestampedHash.h"
#include "catapult/utils/TimeSpan.h"

namespace catapult {
	namespace cache {
		class BasicHashCacheDelta;
		class BasicHashCacheView;
		class HashCache;
		class HashCacheDelta;
		class HashCacheView;

		template<typename TCache, typename TCacheDelta, typename TKey>
		class ReadOnlySimpleCache;
	}
}

namespace catapult { namespace cache {

	/// Describes a hash cache.
	struct HashCacheDescriptor {
	public:
		// key value types
		using KeyType = state::TimestampedHash;
		using ValueType = state::TimestampedHash;

		// cache types
		using CacheType = HashCache;
		using CacheDeltaType = HashCacheDelta;
		using CacheViewType = HashCacheView;

	public:
		/// Gets the key corresponding to \a timestampedHash.
		static const auto& GetKeyFromValue(const ValueType& timestampedHash) {
			return timestampedHash;
		}
	};

	/// Hash cache types.
	struct HashCacheTypes : public ImmutableOrderedSetAdapter<HashCacheDescriptor> {
		using CacheReadOnlyType = ReadOnlySimpleCache<BasicHashCacheView, BasicHashCacheDelta, state::TimestampedHash>;

		/// Custom sub view options.
		struct Options {
			/// Cache retention time.
			utils::TimeSpan RetentionTime;
		};
	};
}}
