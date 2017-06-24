#pragma once
#include "src/state/Namespace.h"
#include "src/state/NamespaceEntry.h"
#include "src/state/RootNamespaceHistory.h"
#include "catapult/deltaset/BaseSet.h"
#include "catapult/utils/Hashers.h"
#include <unordered_map>

namespace catapult {
	namespace cache {
		class BasicNamespaceCacheDelta;
		class BasicNamespaceCacheView;

		template<typename TCache, typename TCacheDelta, typename TKey, typename TGetResult>
		class ReadOnlyArtifactCache;
	}
}

namespace catapult { namespace cache {

	namespace namespace_cache_types {
		/// A read-only view of a namespace cache.
		using CacheReadOnlyType = ReadOnlyArtifactCache<
			BasicNamespaceCacheView,
			BasicNamespaceCacheDelta,
			NamespaceId,
			state::NamespaceEntry>;

		namespace namespace_id_namespace_map {
			/// The map value type.
			using ValueType = state::Namespace;

			/// The entity traits.
			using EntityTraits = deltaset::MutableTypeTraits<ValueType>;

			/// The underlying info map.
			using NamespaceIdBasedNamespaceMap = std::unordered_map<
				NamespaceId,
				ValueType,
				utils::BaseValueHasher<NamespaceId>>;

			/// Retrieves the map key from namespace \a ns.
			struct NamespaceToNamespaceIdConverter {
				static auto ToKey(const ValueType& ns) {
					return ns.id();
				}
			};

			/// The base set type.
			using BaseSetType = deltaset::BaseSet<
				EntityTraits,
				deltaset::MapStorageTraits<NamespaceIdBasedNamespaceMap, NamespaceToNamespaceIdConverter>>;

			/// The base set delta type.
			using BaseSetDeltaType = BaseSetType::DeltaType;
		}

		namespace namespace_id_root_namespace_history_map {
			/// The map value type.
			using ValueType = state::RootNamespaceHistory;

			/// The entity traits.
			using EntityTraits = deltaset::MutableTypeTraits<ValueType>;

			/// The underlying history map.
			using NamespaceIdBasedHistoryMap = std::unordered_map<
				NamespaceId,
				ValueType,
				utils::BaseValueHasher<NamespaceId>>;

			/// Retrieves the map key from \a history.
			struct RootNamespaceHistoryToNamespaceIdConverter {
				static auto ToKey(const ValueType& history) {
					return history.id();
				}
			};

			/// The base set type.
			using BaseSetType = deltaset::BaseSet<
				EntityTraits,
				deltaset::MapStorageTraits<NamespaceIdBasedHistoryMap, RootNamespaceHistoryToNamespaceIdConverter>>;

			/// The base set delta type.
			using BaseSetDeltaType = BaseSetType::DeltaType;
		}
	}
}}
