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
#include "catapult/cache/PatriciaTreeEncoderAdapters.h"
#include "catapult/tree/BasePatriciaTree.h"
#include "catapult/tree/MemoryDataSource.h"
#include <vector>

namespace catapult { namespace test {

	/// Serializer for uint32_t key and std::string value.
	struct MemoryPatriciaTreeSimpleSerializer {
		using KeyType = uint32_t;
		using ValueType = std::string;

		static const std::string& SerializeValue(const ValueType& value) {
			return value;
		}
	};

	/// Memory patricia tree used in cache tests.
	using MemoryPatriciaTree = tree::PatriciaTree<
		cache::SerializerPlainKeyEncoder<MemoryPatriciaTreeSimpleSerializer>,
		tree::MemoryDataSource>;

	/// Memory base patricia tree used in cache tests.
	using MemoryBasePatriciaTree = tree::BasePatriciaTree<
		cache::SerializerPlainKeyEncoder<MemoryPatriciaTreeSimpleSerializer>,
		tree::MemoryDataSource>;

	/// Seeds \a tree with four nodes.
	void SeedTreeWithFourNodes(MemoryPatriciaTree& tree);

	/// Seeds \a tree with four nodes.
	void SeedTreeWithFourNodes(MemoryBasePatriciaTree& tree);

	/// Calculates the expected patricia tree root hash given nodes represented by \a pairs.
	Hash256 CalculateRootHash(const std::vector<std::pair<uint32_t, std::string>>& pairs);
}}
