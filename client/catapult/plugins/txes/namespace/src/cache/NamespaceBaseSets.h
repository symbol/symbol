/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#include "NamespaceCacheSerializers.h"
#include "NamespaceCacheTypes.h"
#include "catapult/cache/CachePatriciaTree.h"
#include "catapult/cache/PatriciaTreeEncoderAdapters.h"
#include "catapult/tree/BasePatriciaTree.h"

namespace catapult { namespace cache {

	using BasicNamespacePatriciaTree = tree::BasePatriciaTree<
		SerializerHashedKeyEncoder<RootNamespaceHistoryPatriciaTreeSerializer>,
		PatriciaTreeRdbDataSource,
		utils::BaseValueHasher<NamespaceId>>;

	class NamespacePatriciaTree : public BasicNamespacePatriciaTree {
	public:
		using BasicNamespacePatriciaTree::BasicNamespacePatriciaTree;
		using Serializer = RootNamespaceHistoryPatriciaTreeSerializer;
	};

	struct NamespaceBaseSetDeltaPointers {
		NamespaceCacheTypes::PrimaryTypes::BaseSetDeltaPointerType pPrimary;
		NamespaceCacheTypes::FlatMapTypes::BaseSetDeltaPointerType pFlatMap;
		NamespaceCacheTypes::HeightGroupingTypes::BaseSetDeltaPointerType pHeightGrouping;
		std::shared_ptr<NamespacePatriciaTree::DeltaType> pPatriciaTree;
	};

	struct NamespaceBaseSets : public CacheDatabaseMixin {
	public:
		/// Indicates the set is not ordered.
		using IsOrderedSet = std::false_type;

	public:
		explicit NamespaceBaseSets(const CacheConfiguration& config)
				: CacheDatabaseMixin(config, { "default", "flat_map", "height_grouping" })
				, Primary(GetContainerMode(config), database(), 0)
				, FlatMap(GetContainerMode(config), database(), 1)
				, HeightGrouping(GetContainerMode(config), database(), 2)
				, PatriciaTree(hasPatriciaTreeSupport(), database(), 3)
		{}

	public:
		NamespaceCacheTypes::PrimaryTypes::BaseSetType Primary;
		NamespaceCacheTypes::FlatMapTypes::BaseSetType FlatMap;
		NamespaceCacheTypes::HeightGroupingTypes::BaseSetType HeightGrouping;
		CachePatriciaTree<NamespacePatriciaTree> PatriciaTree;

	public:
		NamespaceBaseSetDeltaPointers rebase() {
			return { Primary.rebase(), FlatMap.rebase(), HeightGrouping.rebase(), PatriciaTree.rebase() };
		}

		NamespaceBaseSetDeltaPointers rebaseDetached() const {
			return {
				Primary.rebaseDetached(),
				FlatMap.rebaseDetached(),
				HeightGrouping.rebaseDetached(),
				PatriciaTree.rebaseDetached()
			};
		}

		void commit() {
			Primary.commit();
			FlatMap.commit();
			HeightGrouping.commit();
			PatriciaTree.commit();
			flush();
		}
	};
}}
