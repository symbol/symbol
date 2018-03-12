#pragma once
#include "src/state/MosaicEntry.h"
#include "src/state/MosaicHistory.h"
#include "catapult/cache/CacheDescriptorAdapters.h"
#include "catapult/utils/Hashers.h"
#include "catapult/utils/IdentifierGroup.h"

namespace catapult {
	namespace cache {
		class BasicMosaicCacheDelta;
		class BasicMosaicCacheView;
		class MosaicCache;
		class MosaicCacheDelta;
		class MosaicCacheView;

		template<typename TCache, typename TCacheDelta, typename TKey, typename TGetResult>
		class ReadOnlyArtifactCache;
	}
}

namespace catapult { namespace cache {

	/// Describes a mosaic cache.
	struct MosaicCacheDescriptor {
	public:
		// key value types
		using KeyType = MosaicId;
		using ValueType = state::MosaicHistory;

		// cache types
		using CacheType = MosaicCache;
		using CacheDeltaType = MosaicCacheDelta;
		using CacheViewType = MosaicCacheView;

	public:
		/// Gets the key corresponding to \a history.
		static auto GetKeyFromValue(const ValueType& history) {
			return history.id();
		}
	};

	/// Mosaic cache types.
	struct MosaicCacheTypes {
	public:
		using CacheReadOnlyType = ReadOnlyArtifactCache<
			BasicMosaicCacheView,
			BasicMosaicCacheDelta,
			MosaicId,
			const state::MosaicEntry&>;

	// region value adapters

	private:
		template<typename TSource, typename TDest>
		struct ValueAdapter {
			using AdaptedValueType = TDest;

			static TDest& Adapt(TSource& history) {
				return history.back();
			}
		};

	public:
		using ConstValueAdapter = ValueAdapter<const state::MosaicHistory, const state::MosaicEntry>;
		using MutableValueAdapter = ValueAdapter<state::MosaicHistory, state::MosaicEntry>;

	// endregion

	// region secondary descriptors

	private:
		struct NamespaceGroupingTypesDescriptor {
		public:
			using KeyType = NamespaceId;
			using ValueType = utils::IdentifierGroup<MosaicId, NamespaceId, utils::BaseValueHasher<MosaicId>>;

		public:
			static auto GetKeyFromValue(const ValueType& namespaceMosaics) {
				return namespaceMosaics.key();
			}
		};

		struct HeightGroupingTypesDescriptor {
		public:
			using KeyType = Height;
			using ValueType = utils::IdentifierGroup<MosaicId, Height, utils::BaseValueHasher<MosaicId>>;

		public:
			static auto GetKeyFromValue(const ValueType& heightMosaics) {
				return heightMosaics.key();
			}
		};

	// endregion

	public:
		using PrimaryTypes = MutableUnorderedMapAdapter<MosaicCacheDescriptor, utils::BaseValueHasher<MosaicId>>;
		using NamespaceGroupingTypes = MutableUnorderedMapAdapter<NamespaceGroupingTypesDescriptor, utils::BaseValueHasher<NamespaceId>>;
		using HeightGroupingTypes = MutableUnorderedMapAdapter<HeightGroupingTypesDescriptor, utils::BaseValueHasher<Height>>;

	public:
		// in order to compose mosaic cache from multiple sets, define an aggregate set type

		struct BaseSetDeltaPointerType {
			PrimaryTypes::BaseSetDeltaPointerType pPrimary;
			NamespaceGroupingTypes::BaseSetDeltaPointerType pNamespaceGrouping;
			HeightGroupingTypes::BaseSetDeltaPointerType pHeightGrouping;
		};

		struct BaseSetType {
		public:
			PrimaryTypes::BaseSetType Primary;
			NamespaceGroupingTypes::BaseSetType NamespaceGrouping;
			HeightGroupingTypes::BaseSetType HeightGrouping;

		public:
			BaseSetDeltaPointerType rebase() {
				return { Primary.rebase(), NamespaceGrouping.rebase(), HeightGrouping.rebase() };
			}

			BaseSetDeltaPointerType rebaseDetached() const {
				return { Primary.rebaseDetached(), NamespaceGrouping.rebaseDetached(), HeightGrouping.rebaseDetached() };
			}

			void commit() {
				Primary.commit();
				NamespaceGrouping.commit();
				HeightGrouping.commit();
			}
		};
	};
}}
