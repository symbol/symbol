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
#include "LockInfoCacheTypes.h"
#include "catapult/cache/CachePatriciaTree.h"
#include "catapult/cache/PatriciaTreeEncoderAdapters.h"
#include "catapult/tree/BasePatriciaTree.h"

namespace catapult { namespace cache {

	template<typename TDescriptor>
	using LockInfoPatriciaTree = tree::BasePatriciaTree<
		SerializerHashedKeyEncoder<typename TDescriptor::Serializer>,
		PatriciaTreeRdbDataSource,
		utils::ArrayHasher<typename TDescriptor::KeyType>>;

	template<typename TCacheTypes, typename TDescriptor>
	struct LockInfoBaseSetDeltaPointers {
		typename TCacheTypes::PrimaryTypes::BaseSetDeltaPointerType pPrimary;
		typename TCacheTypes::HeightGroupingTypes::BaseSetDeltaPointerType pHeightGrouping;
		std::shared_ptr<typename TDescriptor::PatriciaTree::DeltaType> pPatriciaTree;
	};

	template<typename TCacheTypes, typename TDescriptor, typename TBaseSetDeltaPointers>
	struct LockInfoBaseSets : public CacheDatabaseMixin {
	public:
		/// Indicates the set is not ordered.
		using IsOrderedSet = std::false_type;

	public:
		explicit LockInfoBaseSets(const CacheConfiguration& config)
				: CacheDatabaseMixin(config, { "default", "height_grouping" })
				, Primary(GetContainerMode(config), database(), 0)
				, HeightGrouping(GetContainerMode(config), database(), 1)
				, PatriciaTree(hasPatriciaTreeSupport(), database(), 2)
		{}

	public:
		typename TCacheTypes::PrimaryTypes::BaseSetType Primary;
		typename TCacheTypes::HeightGroupingTypes::BaseSetType HeightGrouping;
		CachePatriciaTree<typename TDescriptor::PatriciaTree> PatriciaTree;

	public:
		TBaseSetDeltaPointers rebase() {
			TBaseSetDeltaPointers deltaPointers;
			deltaPointers.pPrimary = Primary.rebase();
			deltaPointers.pHeightGrouping = HeightGrouping.rebase();
			deltaPointers.pPatriciaTree = PatriciaTree.rebase();
			return deltaPointers;
		}

		TBaseSetDeltaPointers rebaseDetached() const {
			TBaseSetDeltaPointers deltaPointers;
			deltaPointers.pPrimary = Primary.rebaseDetached();
			deltaPointers.pHeightGrouping = HeightGrouping.rebaseDetached();
			deltaPointers.pPatriciaTree = PatriciaTree.rebaseDetached();
			return deltaPointers;
		}

		void commit() {
			Primary.commit();
			HeightGrouping.commit();
			PatriciaTree.commit();
			flush();
		}
	};
}}
