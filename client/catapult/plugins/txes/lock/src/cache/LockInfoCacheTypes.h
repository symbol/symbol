#pragma once
#include "catapult/cache/CacheDescriptorAdapters.h"
#include "catapult/utils/Hashers.h"

namespace catapult { namespace cache {

	/// Basic lock info cache types.
	template<typename TDescriptor>
	struct LockInfoCacheTypes {
	private:
		using IdentifierType = typename TDescriptor::KeyType;

	// region secondary descriptors

	private:
		struct HeightGroupingTypesDescriptor {
		public:
			using KeyType = Height;
			using ValueType = utils::IdentifierGroup<IdentifierType, Height, utils::ArrayHasher<IdentifierType>>;

		public:
			static auto GetKeyFromValue(const ValueType& heightHashes) {
				return heightHashes.key();
			}
		};

	// endregion

	public:
		using PrimaryTypes = MutableUnorderedMapAdapter<TDescriptor, utils::ArrayHasher<IdentifierType>>;
		using HeightGroupingTypes = MutableUnorderedMapAdapter<HeightGroupingTypesDescriptor, utils::BaseValueHasher<Height>>;

	public:
		// in order to compose lock info cache from multiple sets, define an aggregate set type

		struct BaseSetDeltaPointerType {
			typename PrimaryTypes::BaseSetDeltaPointerType pPrimary;
			typename HeightGroupingTypes::BaseSetDeltaPointerType pHeightGrouping;
		};

		struct BaseSetType {
		public:
			typename PrimaryTypes::BaseSetType Primary;
			typename HeightGroupingTypes::BaseSetType HeightGrouping;

		public:
			BaseSetDeltaPointerType rebase() {
				return { Primary.rebase(), HeightGrouping.rebase() };
			}

			BaseSetDeltaPointerType rebaseDetached() const {
				return { Primary.rebaseDetached(), HeightGrouping.rebaseDetached() };
			}

			void commit() {
				Primary.commit();
				HeightGrouping.commit();
			}
		};
	};
}}
