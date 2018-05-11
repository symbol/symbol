/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#pragma once
#include "catapult/cache/CacheDatabaseMixin.h"
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

		struct BaseSetDeltaPointers {
			typename PrimaryTypes::BaseSetDeltaPointerType pPrimary;
			typename HeightGroupingTypes::BaseSetDeltaPointerType pHeightGrouping;
		};

		struct BaseSets : public CacheDatabaseMixin {
		public:
			explicit BaseSets(const CacheConfiguration& config)
					: CacheDatabaseMixin(config, { "default", "height_grouping" })
					, Primary(GetContainerMode(config), database(), 0)
					, HeightGrouping(GetContainerMode(config), database(), 1)
			{}

		public:
			typename PrimaryTypes::BaseSetType Primary;
			typename HeightGroupingTypes::BaseSetType HeightGrouping;

		public:
			BaseSetDeltaPointers rebase() {
				return { Primary.rebase(), HeightGrouping.rebase() };
			}

			BaseSetDeltaPointers rebaseDetached() const {
				return { Primary.rebaseDetached(), HeightGrouping.rebaseDetached() };
			}

			void commit() {
				Primary.commit();
				HeightGrouping.commit();
			}
		};
	};
}}
