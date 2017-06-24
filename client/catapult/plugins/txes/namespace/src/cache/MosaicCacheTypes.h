#pragma once
#include "src/state/GroupedMosaicIds.h"
#include "src/state/MosaicEntry.h"
#include "src/state/MosaicHistory.h"
#include "catapult/deltaset/BaseSet.h"
#include "catapult/utils/Hashers.h"
#include <unordered_map>

namespace catapult {
	namespace cache {
		class BasicMosaicCacheDelta;
		class BasicMosaicCacheView;

		template<typename TCache, typename TCacheDelta, typename TKey, typename TGetResult>
		class ReadOnlyArtifactCache;
	}
}

namespace catapult { namespace cache {
	namespace mosaic_cache_types {
		namespace mosaic_id_mosaic_history_map {
			/// The map value type.
			using ValueType = state::MosaicHistory;

			/// The entity traits.
			using EntityTraits = deltaset::MutableTypeTraits<ValueType>;

			/// The underlying history map.
			using MosaicIdBasedHistoryMap = std::unordered_map<
				MosaicId,
				ValueType,
				utils::BaseValueHasher<MosaicId>>;

			/// Retrieves the map key from \a history.
			struct MosaicHistoryToMosaicIdConverter {
				static auto ToKey(const ValueType& history) {
					return history.id();
				}
			};

			/// The base set type.
			using BaseSetType = deltaset::BaseSet<
				EntityTraits,
				deltaset::MapStorageTraits<MosaicIdBasedHistoryMap, MosaicHistoryToMosaicIdConverter>>;

			/// A pointer to the base set delta type.
			using BaseSetDeltaPointerType = std::shared_ptr<BaseSetType::DeltaType>;
		}

		namespace namespace_id_mosaic_ids_map {
			/// The map value type.
			using ValueType = state::NamespaceMosaics;

			/// The entity traits.
			using EntityTraits = deltaset::MutableTypeTraits<ValueType>;

			/// The underlying mosaic ids map.
			using NamespaceIdBasedMosaicsIdsMap = std::unordered_map<
				NamespaceId,
				ValueType,
				utils::BaseValueHasher<NamespaceId>>;

			/// Retrieves the map key from \a namespaceMosaics.
			struct NamespaceMosaicsToNamespaceIdConverter {
				static auto ToKey(const ValueType& namespaceMosaics) {
					return namespaceMosaics.key();
				}
			};

			/// The base set type.
			using BaseSetType = deltaset::BaseSet<
				EntityTraits,
				deltaset::MapStorageTraits<NamespaceIdBasedMosaicsIdsMap, NamespaceMosaicsToNamespaceIdConverter>>;

			/// A pointer to the base set delta type.
			using BaseSetDeltaPointerType = std::shared_ptr<BaseSetType::DeltaType>;
		}

		namespace height_mosaic_ids_map {
			/// The map value type.
			using ValueType = state::HeightMosaics;

			/// The entity traits.
			using EntityTraits = deltaset::MutableTypeTraits<ValueType>;

			/// The underlying mosaic ids map.
			using HeightBasedMosaicsIdsMap = std::unordered_map<
				Height,
				ValueType,
				utils::BaseValueHasher<Height>>;

			/// Retrieves the map key from \a heightMosaics.
			struct HeightMosaicsToHeightConverter {
				static auto ToKey(const ValueType& heightMosaics) {
					return heightMosaics.key();
				}
			};

			/// The base set type.
			using BaseSetType = deltaset::BaseSet<
				EntityTraits,
				deltaset::MapStorageTraits<HeightBasedMosaicsIdsMap, HeightMosaicsToHeightConverter>>;

			/// A pointer to the base set delta type.
			using BaseSetDeltaPointerType = std::shared_ptr<BaseSetType::DeltaType>;
		}

		/// A read-only view of a mosaic cache.
		using CacheReadOnlyType = ReadOnlyArtifactCache<
			BasicMosaicCacheView,
			BasicMosaicCacheDelta,
			MosaicId,
			const state::MosaicEntry&>;
	}
}}
