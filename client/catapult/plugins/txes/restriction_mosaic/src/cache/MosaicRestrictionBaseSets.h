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
#include "MosaicRestrictionCacheSerializers.h"
#include "MosaicRestrictionCacheTypes.h"
#include "catapult/cache/CachePatriciaTree.h"
#include "catapult/cache/PatriciaTreeEncoderAdapters.h"
#include "catapult/cache/SingleSetCacheTypesAdapter.h"
#include "catapult/tree/BasePatriciaTree.h"

namespace catapult { namespace cache {

	using BasicMosaicRestrictionPatriciaTree = tree::BasePatriciaTree<
		SerializerHashedKeyEncoder<MosaicRestrictionCacheDescriptor::Serializer>,
		PatriciaTreeRdbDataSource,
		utils::ArrayHasher<Hash256>>;

	class MosaicRestrictionPatriciaTree : public BasicMosaicRestrictionPatriciaTree {
	public:
		using BasicMosaicRestrictionPatriciaTree::BasicMosaicRestrictionPatriciaTree;
		using Serializer = MosaicRestrictionCacheDescriptor::Serializer;
	};

	using MosaicRestrictionSingleSetCacheTypesAdapter =
		SingleSetAndPatriciaTreeCacheTypesAdapter<MosaicRestrictionCacheTypes::PrimaryTypes, MosaicRestrictionPatriciaTree>;

	struct MosaicRestrictionBaseSetDeltaPointers : public MosaicRestrictionSingleSetCacheTypesAdapter::BaseSetDeltaPointers {};

	struct MosaicRestrictionBaseSets
			: public MosaicRestrictionSingleSetCacheTypesAdapter::BaseSets<MosaicRestrictionBaseSetDeltaPointers> {
		using MosaicRestrictionSingleSetCacheTypesAdapter::BaseSets<MosaicRestrictionBaseSetDeltaPointers>::BaseSets;
	};
}}
