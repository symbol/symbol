#pragma once
#include "src/state/TimestampedHash.h"
#include "catapult/deltaset/OrderedSet.h"
#include "catapult/utils/NonCopyable.h"

namespace catapult {
	namespace cache {
		class BasicHashCacheDelta;
		class BasicHashCacheView;

		template<typename TCache, typename TCacheDelta, typename TKey>
		class ReadOnlySimpleCache;
	}
}

namespace catapult { namespace cache {

	namespace hash_cache_types {
		/// The cache value type.
		using ValueType = state::TimestampedHash;

		/// The entity traits
		using EntityTraits = deltaset::ImmutableTypeTraits<ValueType>;

		/// The base set type.
		using BaseSetType = deltaset::OrderedSet<EntityTraits>;

		/// A pointer to the base set delta type.
		using BaseSetDeltaPointerType = std::shared_ptr<BaseSetType::DeltaType>;

		/// A read-only view of a hash cache.
		using CacheReadOnlyType = ReadOnlySimpleCache<BasicHashCacheView, BasicHashCacheDelta, ValueType>;
	}
}}
