#pragma once
#include "src/state/MultisigEntry.h"
#include "catapult/deltaset/BaseSet.h"
#include "catapult/utils/Hashers.h"
#include <unordered_map>

namespace catapult {
	namespace cache {
		class BasicMultisigCacheDelta;
		class BasicMultisigCacheView;

		template<typename TCache, typename TCacheDelta, typename TKey, typename TGetResult>
		class ReadOnlyArtifactCache;
	}
}

namespace catapult { namespace cache {

	namespace multisig_cache_types {
		namespace account_multisig_entries_map {
			/// The map value type.
			using ValueType = state::MultisigEntry;

			/// The entity traits.
			using EntityTraits = deltaset::MutableTypeTraits<ValueType>;

			/// The map key type.
			using KeyType = Key;

			/// The underlying entries map.
			using PublicKeyBasedMultisigEntriesMap = std::unordered_map<KeyType, ValueType, utils::ArrayHasher<KeyType>>;

			/// Retrieves the map key from multisig \a entry.
			struct MultisigEntryToPublicKeyConverter {
				static auto ToKey(const ValueType& entry) {
					return entry.key();
				}
			};

			/// The base set type.
			using BaseSetType = deltaset::BaseSet<
				EntityTraits,
				deltaset::MapStorageTraits<PublicKeyBasedMultisigEntriesMap, MultisigEntryToPublicKeyConverter>>;

			/// A pointer to the base set delta type.
			using BaseSetDeltaPointerType = std::shared_ptr<BaseSetType::DeltaType>;
		}

		/// A read-only view of a multisig entry cache.
		using CacheReadOnlyType = ReadOnlyArtifactCache<
				BasicMultisigCacheView,
				BasicMultisigCacheDelta,
				const Key&,
				state::MultisigEntry>;
	}
}}
