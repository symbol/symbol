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
#include "MosaicCacheSerializers.h"
#include "MosaicCacheTypes.h"
#include "catapult/cache/CachePatriciaTree.h"
#include "catapult/cache/PatriciaTreeEncoderAdapters.h"
#include "catapult/tree/BasePatriciaTree.h"

namespace catapult { namespace cache {

	using BasicMosaicPatriciaTree = tree::BasePatriciaTree<
		SerializerHashedKeyEncoder<MosaicHistoryPatriciaTreeSerializer>,
		PatriciaTreeRdbDataSource,
		utils::BaseValueHasher<MosaicId>>;

	class MosaicPatriciaTree : public BasicMosaicPatriciaTree {
	public:
		using BasicMosaicPatriciaTree::BasicMosaicPatriciaTree;
		using Serializer = MosaicHistoryPatriciaTreeSerializer;
	};

	struct MosaicBaseSetDeltaPointers {
		MosaicCacheTypes::PrimaryTypes::BaseSetDeltaPointerType pPrimary;
		MosaicCacheTypes::NamespaceGroupingTypes::BaseSetDeltaPointerType pNamespaceGrouping;
		MosaicCacheTypes::HeightGroupingTypes::BaseSetDeltaPointerType pHeightGrouping;
		std::shared_ptr<MosaicPatriciaTree::DeltaType> pPatriciaTree;
	};

	struct MosaicBaseSets : public CacheDatabaseMixin {
	public:
		/// Indicates the set is not ordered.
		using IsOrderedSet = std::false_type;

	public:
		explicit MosaicBaseSets(const CacheConfiguration& config)
				: CacheDatabaseMixin(config, { "default", "namespace_grouping", "height_grouping" })
				, Primary(GetContainerMode(config), database(), 0)
				, NamespaceGrouping(GetContainerMode(config), database(), 1)
				, HeightGrouping(GetContainerMode(config), database(), 2)
				, PatriciaTree(hasPatriciaTreeSupport(), database(), 3)
		{}

	public:
		MosaicCacheTypes::PrimaryTypes::BaseSetType Primary;
		MosaicCacheTypes::NamespaceGroupingTypes::BaseSetType NamespaceGrouping;
		MosaicCacheTypes::HeightGroupingTypes::BaseSetType HeightGrouping;
		CachePatriciaTree<MosaicPatriciaTree> PatriciaTree;

	public:
		MosaicBaseSetDeltaPointers rebase() {
			return { Primary.rebase(), NamespaceGrouping.rebase(), HeightGrouping.rebase(), PatriciaTree.rebase() };
		}

		MosaicBaseSetDeltaPointers rebaseDetached() const {
			return {
				Primary.rebaseDetached(),
				NamespaceGrouping.rebaseDetached(),
				HeightGrouping.rebaseDetached(),
				PatriciaTree.rebaseDetached()
			};
		}

		void commit() {
			Primary.commit();
			NamespaceGrouping.commit();
			HeightGrouping.commit();
			PatriciaTree.commit();
			flush();
		}
	};
}}
