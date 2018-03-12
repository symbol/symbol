#pragma once
#include "src/state/MultisigEntry.h"
#include "catapult/cache/CacheDescriptorAdapters.h"
#include "catapult/utils/Hashers.h"

namespace catapult {
	namespace cache {
		class BasicMultisigCacheDelta;
		class BasicMultisigCacheView;
		class MultisigCache;
		class MultisigCacheDelta;
		class MultisigCacheView;

		template<typename TCache, typename TCacheDelta, typename TKey, typename TGetResult>
		class ReadOnlyArtifactCache;
	}
}

namespace catapult { namespace cache {

	/// Describes a multisig cache.
	struct MultisigCacheDescriptor {
	public:
		// key value types
		using KeyType = Key;
		using ValueType = state::MultisigEntry;

		// cache types
		using CacheType = MultisigCache;
		using CacheDeltaType = MultisigCacheDelta;
		using CacheViewType = MultisigCacheView;

	public:
		/// Gets the key corresponding to \a entry.
		static const auto& GetKeyFromValue(const ValueType& entry) {
			return entry.key();
		}
	};

	/// Multisig cache types.
	struct MultisigCacheTypes : public MutableUnorderedMapAdapter<MultisigCacheDescriptor, utils::ArrayHasher<Key>> {
		using CacheReadOnlyType = ReadOnlyArtifactCache<
			BasicMultisigCacheView,
			BasicMultisigCacheDelta,
			const Key&,
			const state::MultisigEntry&>;
	};
}}
